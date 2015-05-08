/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes of all First Level General
**      Header SIP stack APIs.
**
*******************************************************************************
**
** FILENAME:
** 	general.h
**
** DESCRIPTION:
**  	This header file contains the protypes of all the First Level General
**      Header manipulation APIs. This header file is to be included by the SU
**      application which wants to use the services of the SIP stack
**
** DATE      NAME           REFERENCE      REASON
** ----      ----           ---------      ------
** 15Dec99   S.Luthra	    Creation
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/
#ifndef __SIP_GENERAL_H_
#define __SIP_GENERAL_H_

#include "sipstruct.h"
#include "sipcommon.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
** Function: sip_getMediaRangeFromAcceptHdr
** Description:gets the Media Range from the Accept header structure
** Parameters:
**			hdr(IN) 			- Sip Accept Header
**			media_range(OUT)	- The media range to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getMediaRangeFromAcceptHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **media_range,SipError *err));


/***********************************************************************
** Function: sip_setMediaRangeInAcceptHdr
** Description: sets the Media Range in the Sip Accept Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Header
**			media_range(IN)		- Media range to set
**			err(OUT)			- Possible error value(see API ref doc)
************************************************************************/
extern SipBool sip_setMediaRangeInAcceptHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *media_range, SipError *err));


/***********************************************************************
** Function: sip_getAcceptRangeFromAcceptHdr
** Description: gets the Accept Range from the Sip Accept Header
** Parameters:
**			hdr(IN) 			- Sip Accept Header
**			accept_range(OUT)	- Accept range to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptRangeFromAcceptHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit ** accept_range, SipError *err));


/***********************************************************************
** Function: sip_setAcceptRangeInAcceptHdr
** Description: sets the accept range in the Sip Accept Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Header
**			accept_range(IN)	- Accept range to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAcceptRangeInAcceptHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *accept_range, SipError *err));


/***********************************************************************
** Function: sip_getCodingsFromAcceptEncodingHdr
** Description: gets Codings from the Accept Encoding structure
** Parameters:
**			hdr(IN) 			- Sip Accept Header
**			codings(OUT)		- Codings to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getCodingsFromAcceptEncodingHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **codings, SipError *err));


/***********************************************************************
** Function: sip_setCodingsInAcceptEncodingHdr
** Description: sets Codings in the Accept Encoding structure
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Encoding Header
**			codings(IN)			- Codings to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setCodingsInAcceptEncodingHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *codings, SipError *err));


/***********************************************************************
** Function: sip_getQValuesFromAcceptEncodingHdr
** Description: gets QValues from the Accept Encoding structure
** Parameters:
**			hdr(IN) 			- Sip Accept Header
**			qvalues(OUT)			- QValues to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getQValuesFromAcceptEncodingHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **qvalues, SipError *err));


/***********************************************************************
** Function: sip_setQValuesInAcceptEncodingHdr
** Description: sets QValues in the Accept Encoding structure
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Encoding Header
**			qvalues(IN)			- QValues to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setQValuesInAcceptEncodingHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *qvalues, SipError *err));


/***********************************************************************
** Function: sip_getAcceptParamCountFromAcceptEncodingHdr
** Description:gets the number of Accept Parameters from the 
**			   Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			count(IN)			- The number of Accept Parameters is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamCountFromAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr, SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAcceptParamAtIndexFromAcceptEncodingHdr
** Description:gets the Accept Parameter at an index from the
** 			   Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			pAcceptParam(OUT)	- The Accept Param is retrieved
**			index(IN)			- Index at which the Accept Param is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamAtIndexFromAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError  *err));
#else
/***********************************************************************
** Function: sip_getAcceptParamAtIndexFromAcceptEncodingHdr
** Description:gets the Accept Parameter at an index from the 
**				Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			pAcceptParam(OUT)	- The Accept Parameter that is retrieved
**			index(IN)			- Index at which the Accept Param is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamAtIndexFromAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair **ppAcceptParam,\
		SIP_U32bit index, SipError  *err));
#endif

/***********************************************************************
** Function: sip_insertAcceptParamAtIndexInAcceptEncodingHdr
** Description:inserts the Accept Parameter at an index in the 
**				Sip Accept Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			pAcceptParam(IN		- The Accept Param that is inserted
**			index(IN)			- Index at which the Accept Param is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertAcceptParamAtIndexInAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError *err));


/***********************************************************************
** Function: sip_setAcceptParamAtIndexInAcceptEncodingHdr
** Description:sets the Accept Parameter at an index in the Sip Accept 
**				Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			pAcceptParam(IN)	- The Accept Parameter that is set
**			index(IN)			- Index at which the Accept Param is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAcceptParamAtIndexInAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr, SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError *err));


/***********************************************************************
** Function: sip_deleteAcceptParamAtIndexInAcceptEncodingHdr
** Description:deletes the Accept Parameter at an index in the Sip Accept
**				Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			index(IN)			- Index from which the Accept Param is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteAcceptParamAtIndexInAcceptEncodingHdr _ARGS_\
	((SipHeader *hdr, SIP_U32bit index, SipError *err));

	
/***********************************************************************
** Function: sip_getLangRangeFromAcceptLangHdr
** Description:gets the Language Range from the Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			lang_range(OUT)		- Language range to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getLangRangeFromAcceptLangHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **lang_range, SipError *err));


/***********************************************************************
** Function: sip_setLangRangeInAcceptLangHdr
** Description:sets the Language Range in the Accept Language Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Language Header
**			lang_range(IN)		- Language range to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setLangRangeInAcceptLangHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *lang_range, SipError *err));


/***********************************************************************
** Function: sip_getQvalueFromAcceptLangHdr
** Description: gets the Q value field from the Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			qvalue(OUT)			- Q value to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getQvalueFromAcceptLangHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **qvalue, SipError *err));


/***********************************************************************
** Function: sip_setQvalueInAcceptLangHdr
** Description: sets the Q value field in the Accept Language Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Accept Language Header
**			qvalue(IN)			- Q value to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setQvalueInAcceptLangHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *qvalue, SipError *err));


/***********************************************************************
** Function: sip_getAcceptParamCountFromAcceptLangHdr
** Description:gets the number of Accept Parameters from the 
**			   Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			count(IN)			- The number of Accept Parameters is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamCountFromAcceptLangHdr _ARGS_\
	((SipHeader *hdr, SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAcceptParamAtIndexFromAcceptLangHdr
** Description:gets the Accept Parameter at an index from the
** 			   Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			pAcceptParam(OUT)	- The Accept Param is retrieved
**			index(IN)			- Index at which the Accept Param is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamAtIndexFromAcceptLangHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError  *err));
#else
/***********************************************************************
** Function: sip_getAcceptParamAtIndexFromAcceptLangHdr
** Description:gets the Accept Parameter at an index from the 
**				Sip Accept Language Header
** Parameters:
**			hdr(IN) 			- Sip Accept Language Header
**			pAcceptParam(OUT)	- The Accept Parameter that is retrieved
**			index(IN)			- Index at which the Accept Param is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAcceptParamAtIndexFromAcceptLangHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair **ppAcceptParam,\
		SIP_U32bit index, SipError  *err));
#endif

/***********************************************************************
** Function: sip_insertAcceptParamAtIndexInAcceptLangHdr
** Description:inserts the Accept Parameter at an index in the 
**				Sip Accept Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			pAcceptParam(IN		- The Accept Param that is inserted
**			index(IN)			- Index at which the Accept Param is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertAcceptParamAtIndexInAcceptLangHdr _ARGS_\
	((SipHeader *hdr,SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError *err));


/***********************************************************************
** Function: sip_setAcceptParamAtIndexInAcceptLangHdr
** Description:sets the Accept Parameter at an index in the Sip Accept 
**				Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			pAcceptParam(IN)	- The Accept Parameter that is set
**			index(IN)			- Index at which the Accept Param is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAcceptParamAtIndexInAcceptLangHdr _ARGS_\
	((SipHeader *hdr, SipNameValuePair *pAcceptParam, SIP_U32bit index,\
	SipError *err));


/***********************************************************************
** Function: sip_deleteAcceptParamAtIndexInAcceptLangHdr
** Description:deletes the Accept Parameter at an index in the Sip Accept
**				Language Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Accept Language Header
**			index(IN)			- Index from which the Accept Param is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteAcceptParamAtIndexInAcceptLangHdr _ARGS_\
	((SipHeader *hdr, SIP_U32bit index, SipError *err));

/***********************************************************************
** Function:sip_getValueFromCallIdHdr
** Description:gets the Value from the CallId Header
** Parameters:
**			hdr(IN) 			- Sip CallId Header
**			value(IN)			- value to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getValueFromCallIdHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **value, SipError *err));


/***********************************************************************
** Function: sip_setValueInCallIdHdr
** Description:sets the Value in the CallId Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CallId Header
**			value(IN)			- value to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setValueInCallIdHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *value, SipError *err));


/***********************************************************************
** Function: sip_getTypeFromContactHdr
** Description:gets the type from the Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			type(OUT)			- type to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTypeFromContactHdr _ARGS_ ((SipHeader *hdr, \
	en_ContactType *type, SipError *err));


/***********************************************************************
** Function: sip_setTypeInContactHdr
** Description:sets the type in the Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			type(IN)			- type to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTypeInContactHdr _ARGS_ ((SipHeader *hdr, \
	en_ContactType  type, SipError *err));


/***********************************************************************
** Function: sip_getDispNameFromContactHdr
** Description:gets the display name from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			disp_name(OUT)		- display name to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **disp_name, SipError *err));


/***********************************************************************
** Function: sip_setDispNameInContactHdr
** Description:sets the display name in the Sip Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			disp_name(IN)		- display name to set
**			err(OUT)			- Possible error value(see API ref doc)
************************************************************************/
extern SipBool sip_setDispNameInContactHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *disp_name, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromContactHdr
** Description:gets the Sip Addr Spec structure from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			pAddrSpec(OUT)		- Addr Spec structure to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec *pAddrSpec, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromContactHdr
** Description:gets the Sip Addr Spec structure from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			pAddrSpec(OUT)		- Addr Spec structure to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec **ppAddrSpec, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInContactHdr
** Description:sets the Sip Addr Spec structure in the Sip Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			pAddrSpec(IN)		- Addr Spec structure to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInContactHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec *pAddrSpec, SipError *err));




#define sip_getContactParamCountFromContactHdr sip_getContactParamsCountFromContactHdr

/***********************************************************************
** Function: sip_getContactParamsCountFromContactHdr
** Description:gets the number of Contact Parameters from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			count(OUT)			- Count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getContactParamsCountFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getContactParamAtIndexFromContactHdr
** Description:gets the Contact Parameters at an index from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			conatct_param(OUT)	- ContactParameter to retrieve
**			index(IN)			- Index at which the Contact Param is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getContactParamAtIndexFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SipContactParam *contact_param, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getContactParamAtIndexFromContactHdr
** Description:gets the Contact Parameters at an index from the Sip Contact Header
** Parameters:
**			hdr(IN) 			- Sip Contact Header
**			conatct_param(OUT)	- ContactParameter to retrieve
**			index(IN)			- Index at which the Contact Param is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getContactParamAtIndexFromContactHdr _ARGS_ ((SipHeader *hdr, \
	SipContactParam **ppContactParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertContactParamAtIndexInContactHdr
** Description:insert the Contact Parameters at an index in the Sip Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			conatct_param(IN)	- ContactParameter to insert
**			index(IN)			- Index at which the Contact Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertContactParamAtIndexInContactHdr _ARGS_ ((SipHeader *hdr, \
	SipContactParam *contact_param, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setContactParamAtIndexInContactHdr
** Description:set the Contact Parameters at an index in the Sip Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			conatct_param(IN)	- ContactParameter to set
**			index(IN)			- Index at which the Contact Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setContactParamAtIndexInContactHdr _ARGS_ ((SipHeader *hdr, \
	SipContactParam *contact_param, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteContactParamAtIndexInContactHdr
** Description:delete the Contact Parameter at an index in the Sip Contact Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Contact Header
**			index(IN)			- Index at which the Contact Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteContactParamAtIndexInContactHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getSeqNumFromCseqHdr
** Description:get the Sequence number from the Sip CSequence Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CSequence Header
**			seq_no(OUT)			- The sequencenumber retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSeqNumFromCseqHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit *seq_no, SipError *err));


/***********************************************************************
** Function: sip_setSeqNumInCseqHdr
** Description:set the Sequence number in the Sip CSequence Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CSequence Header
**			seq_no(IN)			- The sequencenumber set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSeqNumInCseqHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit seq_no, SipError *err));


/***********************************************************************
** Function: sip_getMethodFromCseqHdr
** Description:get the Method from the Sip CSequence Header
** Parameters:
**			hdr(IN) 			- Sip CSequence Header
**			method(OUT)			- The method to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getMethodFromCseqHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **method, SipError *err));


/***********************************************************************
** Function: sip_setMethodInCseqHdr
** Description:set the Method in the Sip CSequence Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CSequence Header
**			method(IN)			- The method to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setMethodInCseqHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit * method, SipError *err));


/***********************************************************************
** Function: sip_getDayOfWeekFromDateHdr
** Description:gets the Day of the Week from the Sip Date Header
** Parameters:
**			hdr(IN) 			- Sip Date Header
**			day(OUT)			- The Day of the Week to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDayOfWeekFromDateHdr _ARGS_ ((SipHeader *hdr, \
		en_DayOfWeek *day, SipError *err));


/***********************************************************************
** Function: sip_setDayOfWeekInDateHdr
** Description:sets the Day of the Week in the Sip Date Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Date Header
**			day(IN)				- The Day of the Week to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDayOfWeekInDateHdr _ARGS_ ((SipHeader *hdr, en_DayOfWeek day, \
	SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getDateFormatFromDateHdr
** Description:gets the Date Format from the Sip Date Header
** Parameters:
**			hdr(IN) 			- Sip Date Header
**			date(OUT)			- The Date Format to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateFormatFromDateHdr _ARGS_ ((SipHeader *hdr, \
	SipDateFormat *date, SipError *err));

#else
/***********************************************************************
** Function: sip_getDateFormatFromDateHdr
** Description:gets the Date Format from the Sip Date Header
** Parameters:
**			hdr(IN) 			- Sip Date Header
**			date(OUT)			- The Date Format to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateFormatFromDateHdr _ARGS_ ((SipHeader *hdr, \
	SipDateFormat **ppDate, SipError *err));

#endif

/***********************************************************************
** Function: sip_setDateFormatInDateHdr
** Description:sets the Date Format in the Sip Date Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Date Header
**			date(IN)			- The Date Format to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDateFormatInDateHdr _ARGS_ ((SipHeader *hdr, \
	SipDateFormat *date, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getTimeFormatFromDateHdr
** Description:gets the Time Format from the Sip Date Header
** Parameters:
**			hdr(IN) 			- Sip Date Header
**			Time(OUT)			- The Time Format to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTimeFormatFromDateHdr _ARGS_ ((SipHeader *hdr, \
	SipTimeFormat *time, SipError *err));

#else
/***********************************************************************
** Function: sip_getTimeFormatFromDateHdr
** Description:gets the Time Format from the Sip Date Header
** Parameters:
**			hdr(IN) 			- Sip Date Header
**			Time(OUT)			- The Time Format to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTimeFormatFromDateHdr _ARGS_ ((SipHeader *hdr, \
	SipTimeFormat **ppTime, SipError *err));

#endif

/***********************************************************************
** Function: sip_setTimeFormatInDateHdr
** Description:sets the Time Format in the Sip Date Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Date Header
**			Time(IN)			- The Time Format to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTimeFormatInDateHdr _ARGS_ ((SipHeader *hdr, \
		SipTimeFormat *time, SipError *err));


/***********************************************************************
** Function: sip_getSchemeFromEncryptionHdr
** Description:gets the Scheme from the Sip Encryption Header
** Parameters:
**			hdr(IN) 			- Sip Encryption Header
**			scheme(OUT)			- The Scheme to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSchemeFromEncryptionHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **scheme, SipError *err));


/***********************************************************************
** Function: sip_setSchemeInEncryptionHdr
** Description:sets the Scheme in the Sip Encryption Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Encryption Header
**			scheme(IN)			- The Scheme to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSchemeInEncryptionHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *scheme, SipError *err));


/***********************************************************************
** Function: sip_getParamCountFromEncryptionHdr
** Description:gets the number of Parameters from the Sip Encryption Header
** Parameters:
**			hdr(IN) 			- Sip Encryption Header
**			count(OUT)			- The Parameter count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromEncryptionHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromEncryptionHdr
** Description:gets the Parameters at an index from the Sip Encryption Header
** Parameters:
**			hdr(IN) 			- Sip Encryption Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromEncryptionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromEncryptionHdr
** Description:gets the Parameter at an index from the Sip Encryption Header
** Parameters:
**			hdr(IN) 			- Sip Encryption Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromEncryptionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam **ppParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInEncryptionHdr
** Description:inserts the Parameter at an index in the Sip Encryption Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Encryption Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInEncryptionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteParamAtIndexInEncryptionHdr
** Description:deletes the Parameter at an index in the Sip Encryption Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Encryption Header
**			index(IN)			- The index at which the Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInEncryptionHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setParamAtIndexInEncryptionHdr
** Description:sets the Parameter at an index in the Sip Encryption Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Encryption Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInEncryptionHdr _ARGS_ ((SipHeader *hdr, SipParam *pParam, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getTypeFromExpiresHdr
** Description:gets the Type from the Sip Expires Header
** Parameters:
**			hdr(IN) 			- Sip Expires Header
**			type(OUT)			- The type to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTypeFromExpiresHdr _ARGS_ ((SipHeader *hdr, \
	en_ExpiresType *type, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getDateStructFromExpiresHdr
** Description:gets the Date Structure from the Sip Expires Header
** Parameters:
**			hdr(IN) 			- Sip Expires Header
**			pDateStruct(OUT)	- The date structure to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateStructFromExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SipDateStruct *pDateStruct, SipError *err));

#else
/***********************************************************************
** Function: sip_getDateStructFromExpiresHdr
** Description:gets the Date Structure from the Sip Expires Header
** Parameters:
**			hdr(IN) 			- Sip Expires Header
**			pDateStruct(OUT)	- The date structure to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateStructFromExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SipDateStruct **ppDateStruct, SipError *err));

#endif

/***********************************************************************
** Function: sip_setDateStructInExpiresHdr
** Description:sets the Date Structure in the Sip Expires Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Expires Header
**			pDateStruct(IN)		- The date structure to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDateStructInExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SipDateStruct *date_struct, SipError *err));


/***********************************************************************
** Function: sip_getSecondsFromExpiresHdr
** Description:gets the Seconds field from the Sip Expires Header
** Parameters:
**			hdr(IN) 			- Sip Expires Header
**			sec(OUT)			- The seconds field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSecondsFromExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit *sec, SipError *err));


/***********************************************************************
** Function: sip_setSecondsInExpiresHdr
** Description:sets the Seconds field in the Sip Expires Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Expires Header
**			sec(IN)				- The seconds field to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSecondsInExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit sec, SipError *err));


/***********************************************************************
** Function: sip_getDispNameFromFromHdr
** Description:gets the Display Name from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			name(OUT)			- The Display Name  to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **name, SipError *err));


/***********************************************************************
** Function: sip_setDispNameInFromHdr
** Description:sets the Display Name in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			name(IN)			- The Display Name  to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *name, SipError *err));


/***********************************************************************
** Function: sip_getTagCountFromFromHdr
** Description:gets the Number of Tags from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			count(OUT)			- The Tag count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTagCountFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


/***********************************************************************
** Function: sip_getTagAtIndexFromFromHdr
** Description:gets the Tag at an index from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			tag(OUT)			- The Tag to retrieve
**			index(IN)			- The index at which the tag is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTagAtIndexFromFromHdr _ARGS_ ((SipHeader *hdr,\
		SIP_S8bit **tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_insertTagAtIndexInFromHdr
** Description:inserts a Tag at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			tag(IN)				- The Tag to insert
**			index(IN)			- The index at which the tag is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertTagAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setTagAtIndexInFromHdr
** Description:sets a Tag at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			tag(IN)				- The Tag to set
**			index(IN)			- The index at which the tag is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTagAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteTagAtIndexInFromHdr
** Description:deletes a Tag at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			index(IN)			- The index at which the tag is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteTagAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getExtensionParamCountFromFromHdr
** Description:gets the number of Extension Parameters from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			count(OUT)			- The number of Extension Parameters to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamCountFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getExtensionParamAtIndexFromFromHdr
** Description:gets the Extension Parameter at an index from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			pExtParam(OUT)		- The extension parameter which is retrieved
**			index(IN)			- The index at which the Extension Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamAtIndexFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getExtensionParamAtIndexFromFromHdr
** Description:gets the Extension Parameter at an index from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			pExtParam(OUT)		- The extension paramter which is retrieved
**			index(IN)			- The index at which the Extension Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamAtIndexFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SipParam **ppExtParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertExtensionParamAtIndexInFromHdr
** Description:inserts the Extension Parameter at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			pExtParam(IN)		- The extension paramter which is inserted
**			index(IN)			- The index at which the Extension Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertExtensionParamAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setExtensionParamAtIndexInFromHdr
** Description:sets the Extension Parameter at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			pExtParam(IN)		- The extension paramter which is set
**			index(IN)			- The index at which the Extension Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setExtensionParamAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteExtensionParamAtIndexInFromHdr
** Description:deletes the Extension Parameter at an index in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			index(IN)			- The index at which the Extension Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteExtensionParamAtIndexInFromHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit index, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromFromHdr
** Description:gets the Addr Spec structure from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			pAddrSpec(OUT)		- The AddrSpec structure which is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *pAddrSpec, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromFromHdr
** Description:gets the Addr Spec structure from the Sip From Header
** Parameters:
**			hdr(IN) 			- Sip From Header
**			pAddrSpec(OUT)		- The AddrSpec structure which is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromFromHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec **ppAddrSpec, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInFromHdr
** Description:sets the Addr Spec structure in the Sip From Header
** Parameters:
**			hdr(IN/OUT) 			- Sip From Header
**			pAddrSpec(IN)		- The AddrSpec structure which is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInFromHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *addr_spec, SipError *err));


/***********************************************************************
** Function: sip_getSentByFromViaHdr
** Description:gets the SentBy field from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			sent(OUT)			- The sentBy field which is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSentByFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **sent_by, SipError *err));


/***********************************************************************
** Function: sip_setSentByInViaHdr
** Description:sets the SentBy field in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			sent(IN)			- The sentBy field which is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSentByInViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *sent_by, SipError *err));


/***********************************************************************
** Function: sip_getSentProtocolFromViaHdr
** Description:gets the Sent Protocol from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			sentProtocol(OUT)	- The sentProtocol which is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSentProtocolFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **sent_protocol, SipError *err));


/***********************************************************************
** Function: sip_setSentProtocolInViaHdr
** Description:sets the Sent Protocol in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			sentProtocol(IN)	- The sentProtocol to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSentProtocolInViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *sent_protocol, SipError *err));


/***********************************************************************
** Function: sip_getViaParamCountFromViaHdr
** Description:gets the number of Via Parameters from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			count(IN)			- The number of Via Parameters is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getViaParamCountFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getViaParamAtIndexFromViaHdr
** Description:gets the Via Parameter at an index from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			pViaParam(OUT)		- The Via Parameter is retrieved
**			index(IN)			- Index at which the Via Parameter is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getViaParamAtIndexFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pViaParam, SIP_U32bit index, SipError  *err));

#else
/***********************************************************************
** Function: sip_getViaParamAtIndexFromViaHdr
** Description:gets the Via Parameter at an index from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			pViaParam(OUT)		- The Via Parameter is retrieved
**			index(IN)			- Index at which the Via Parameter is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getViaParamAtIndexFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SipParam **ppViaParam, SIP_U32bit index, SipError  *err));

#endif

/***********************************************************************
** Function: sip_insertViaParamAtIndexInViaHdr
** Description:inserts the Via Parameter at an index in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			pViaParam(IN)		- The Via Parameter is inserted
**			index(IN)			- Index at which the Via Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertViaParamAtIndexInViaHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pViaParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setViaParamAtIndexInViaHdr
** Description:sets the Via Parameter at an index in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			pViaParam(IN)		- The Via Parameter is set
**			index(IN)			- Index at which the Via Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setViaParamAtIndexInViaHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pViaParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteViaParamAtIndexInViaHdr
** Description:deletes the Via Parameter at an index in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			index(IN)			- Index at which the Via Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteViaParamAtIndexInViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getCommentFromViaHdr
** Description:gets the Comment from the Sip Via Header
** Parameters:
**			hdr(IN) 			- Sip Via Header
**			Comment(OUT)		- Comment to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getCommentFromViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **comment, SipError *err));


/***********************************************************************
** Function: sip_setCommentInViaHdr
** Description:sets the Comment in the Sip Via Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Via Header
**			Comment(IN)			- Comment to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setCommentInViaHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *comment, SipError *err));


/***********************************************************************
** Function: sip_getDispNameFromRecordRouteHdr
** Description:gets the Display Name from the Sip Record Route Header
** Parameters:
**			hdr(IN) 			- Sip Record Route Header
**			name(OUT)			- Display Name  to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromRecordRouteHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **name, SipError *err));


/***********************************************************************
** Function: sip_setDispNameInRecordRouteHdr
** Description:sets the Display Name in the Sip Record Route Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Record Route Header
**			name(IN)			- Display Name  to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInRecordRouteHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *name, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromRecordRouteHdr
** Description:gets the Addr Spec from the Sip Record Route Header
** Parameters:
**			hdr(IN) 			- Sip Record Route Header
**			pAddrSpec(OUT)		- Addr Spec to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromRecordRouteHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *pAddrSpec, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromRecordRouteHdr
** Description:gets the Addr Spec from the Sip Record Route Header
** Parameters:
**			hdr(IN) 			- Sip Record Route Header
**			pAddrSpec(OUT)		- Addr Spec to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromRecordRouteHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec **ppAddrSpec, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInRecordRouteHdr
** Description:sets the Addr Spec in the Sip Record Route Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Record Route Header
**			pAddrSpec(IN)		- Addr Spec to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInRecordRouteHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *addr_spec, SipError *err));


/***********************************************************************
** Function:  sip_getParamCountFromRecordRouteHdr
** Description:Retrieve the number of parameters in Record-Route header
** Parameters:
**			pHdr(IN) 			- Sip Record Route Header
**			pCount(OUT)			- count of params
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromRecordRouteHdr
** Description: Retrieves the Param at a specified in a Record-Route header
** Parameters:
**			pHdr(IN) 			- Sip Record Route Header
**
**			pParam/ppParam (OUT)		_ The retrieved param
**			dIndex(IN)			- index of the param to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInRecordRouteHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Record Route Header
**
**			pParam (IN)			_ The param to be set
**			dIndex(IN)			- index at which the param is to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInRecordRouteHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Record Route Header
**
**			pParam (IN)			_ The param to be inserted
**			dIndex(IN)			- index at which the param is to be inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInRecordRouteHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Record Route Header
**			dIndex(IN)			- index at which the param is to be deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInRecordRouteHdr _ARGS_ ((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_getTimeFromTimeStampHdr
** Description:gets the Time from  the Sip Time Stamp Header
** Parameters:
**			hdr(IN) 			- Sip Time Stamp Header
**			time(OUT)			- Time value to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTimeFromTimeStampHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **time, SipError *err));


/***********************************************************************
** Function: sip_setTimeInTimeStampHdr
** Description:sets the Time in  the Sip Time Stamp Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Time Stamp Header
**			time(IN)			- Time value to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTimeInTimeStampHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *time, SipError *err));


/***********************************************************************
** Function: sip_getDelayFromTimeStampHdr
** Description:gets the value of Delay from  the Sip Time Stamp Header
** Parameters:
**			hdr(IN) 			- Sip Time Stamp Header
**			Delay(OUT)			- Delay value to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDelayFromTimeStampHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **delay, SipError *err));


/***********************************************************************
** Function: sip_setDelayInTimeStampHdr
** Description:sets the Delay field  in the Sip Time Stamp Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Time Stamp Header
**			delay(IN)			- Delay value to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDelayInTimeStampHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *delay, SipError *err));


/***********************************************************************
** Function: sip_getDispNameFromToHdr
** Description:gets the Display Name from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			name(OUT)			- Name value to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **name, SipError *err));


/***********************************************************************
** Function: sip_setDispNameInToHdr
** Description:sets the Display Name in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			name(IN)			- Name value to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *name, SipError *err));


/***********************************************************************
** Function: sip_getTagCountFromToHdr
** Description:gets the number of Tags from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			count(OUT)			- NUmber of Tags to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTagCountFromToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


/***********************************************************************
** Function: sip_getTagAtIndexFromToHdr
** Description:gets the Tag at an index from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			tag(OUT)			- Tag value retrieved
**			index(IN)			- The index at which the tag value is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTagAtIndexFromToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_insertTagAtIndexInToHdr
** Description:inserts the Tag at an index in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			tag(IN)				- Tag value to insert
**			index(IN)			- The index at which the tag value is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertTagAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setTagAtIndexInToHdr
** Description:sets the Tag at an index in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			tag(IN)				- Tag value set
**			index(IN)			- The index at which the tag value is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTagAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *tag, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteTagAtIndexInToHdr
** Description:deletes the Tag at an index in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			index(IN)			- The index at which the tag value is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteTagAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getExtensionParamCountFromToHdr
** Description:gets the number of Extension Parameters from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			count(IN)			- The number of Extension Parameters retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamCountFromToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getExtensionParamAtIndexFromToHdr
** Description:gets the Extension Parameter at an index from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			pExtParam(OUT)		- Extension Parameter value to retrieved
**			index(IN)			- The index at which the ExtensionParameter value is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamAtIndexFromToHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getExtensionParamAtIndexFromToHdr
** Description:gets the Extension Parameter at an index from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			pExtParam(OUT)		- Extension Parameter value to retrieved
**			index(IN)			- The index at which the ExtensionParameter value is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getExtensionParamAtIndexFromToHdr _ARGS_ ((SipHeader *hdr, \
		SipParam **ppExtParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertExtensionParamAtIndexInToHdr
** Description:inserts the Extension Parameter at an index in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			pExtParam(IN)		- Extension Parameter value to inserted
**			index(IN)			- The index at which the ExtensionParameter value is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertExtensionParamAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setExtensionParamAtIndexInToHdr
** Description:sets the Extension Parameter at an index from the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			pExtParam(IN)		- Extension Parameter value to set
**			index(IN)			- The index at which the ExtensionParameter value is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setExtensionParamAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SipParam *pExtParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteExtensionParamAtIndexInToHdr
** Description:deletes the Extension Parameter at an index in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			index(IN)			- The index at which the ExtensionParameter value is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteExtensionParamAtIndexInToHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit index, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromToHdr
** Description:gets the Addr Spec from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			paddrSpec(IN)		- The Addr Spec which is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromToHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *pAddrSpec, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromToHdr
** Description:gets the Addr Spec from the Sip To Header
** Parameters:
**			hdr(IN) 			- Sip To Header
**			paddrSpec(IN)		- The Addr Spec which is retrieved.
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromToHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec **ppAddrSpec, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInToHdr
** Description:sets the Addr Spec in the Sip To Header
** Parameters:
**			hdr(IN/OUT) 			- Sip To Header
**			paddrSpec(IN)		- The Addr Spec to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInToHdr _ARGS_ ((SipHeader *hdr, \
		SipAddrSpec *addr_spec, SipError *err));


/***********************************************************************
** Function: sip_getNameFromUnknownHdr
** Description:gets the Name field from the Sip Unknown Header
** Parameters:
**			hdr(IN) 			- Sip Unknown Header
**			name(OUT)			- The name to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getNameFromUnknownHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **name, SipError *err));


/***********************************************************************
** Function: sip_setNameInUnknownHdr
** Description:sets the Name field in the Sip Unknown Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Unknown Header
**			name(IN)			- The name to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setNameInUnknownHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *name, SipError *err));


/***********************************************************************
** Function: sip_getBodyFromUnknownHdr
** Description:gets the Body field from the Sip Unknown Header
** Parameters:
**			hdr(IN) 			- Sip Unknown Header
**			body(OUT)			- The body to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getBodyFromUnknownHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit **body, SipError *err));


/***********************************************************************
** Function: sip_setBodyInUnknownHdr
** Description:sets the Body field in the Sip Unknown Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Unknown Header
**			body(IN)			- The body to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setBodyInUnknownHdr _ARGS_ ((SipHeader *hdr, \
		SIP_S8bit *body, SipError *err));


/***********************************************************************
** Function: sip_getVersionFromStatusLine
** Description:gets the Version field from the Sip Status Line
** Parameters:
**			line(IN) 			- Sip Status Line
**			version(OUT)		- The version to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getVersionFromStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_S8bit **version, SipError *err));


/***********************************************************************
** Function: sip_setVersionInStatusLine
** Description:sets the Version field in the Sip Status Line
** Parameters:
**			line(IN/OUT) 			- Sip Status Line
**			version(IN)			- The version to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setVersionInStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_S8bit *version, SipError *err));


/***********************************************************************
** Function: sip_getStatusCodeFromStatusLine
** Description:gets the Status code field from the Sip Status Line
** Parameters:
**			line(IN) 			- Sip Status Line
**			code(OUT)			- The Status code to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getStatusCodeFromStatusLine _ARGS_ ((SipStatusLine *line, \
		en_StatusCode *code, SipError *err));


/***********************************************************************
** Function: sip_setStatusCodeInStatusLine
** Description:sets the Status code field in the Sip Status Line
** Parameters:
**			line(IN/OUT) 			- Sip Status Line
**			code(IN)			- The code to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setStatusCodeInStatusLine _ARGS_ ((SipStatusLine *line, \
		en_StatusCode code, SipError *err));


/***********************************************************************
** Function: sip_getStatusCodeNumFromStatusLine
** Description:gets the Status code number from the Sip Status Line
** Parameters:
**			line(IN) 			- Sip Status Line
**			code_num(OUT)		- The Status code number to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getStatusCodeNumFromStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_U16bit *code_num, SipError *err));


/***********************************************************************
** Function: sip_setStatusCodeNumInStatusLine
** Description:sets the Status code number in the Sip Status Line
** Parameters:
**			line(IN/OUT) 			- Sip Status Line
**			code_num(IN)		- The Status code number to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setStatusCodeNumInStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_U16bit code_num, SipError *err));


/***********************************************************************
** Function: sip_getReasonFromStatusLine
** Description:gets the Reason in the Sip Status Line
** Parameters:
**			line(IN) 			- Sip Status Line
**			reason(OUT)			- The reason to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getReasonFromStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_S8bit **reason, SipError *err));


/***********************************************************************
** Function: sip_setReasonInStatusLine
** Description:sets the Reason in the Sip Status Line
** Parameters:
**			line(IN/OUT) 			- Sip Status Line
**			reason(IN)			- The reason to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setReasonInStatusLine _ARGS_ ((SipStatusLine *line, \
		SIP_S8bit *reason, SipError *err));


/***********************************************************************
** Function: sip_getMethodFromReqLine
** Description:gets the Method from the Sip Status Line
** Parameters:
**			line(IN) 			- Sip Status Line
**			method(OUT)			- The method to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getMethodFromReqLine _ARGS_ ((SipReqLine *line, \
		SIP_S8bit **method, SipError *err));


/***********************************************************************
** Function: sip_setMethodInReqLine
** Description:sets the Method in the Sip Status Line
** Parameters:
**			line(IN/OUT) 			- Sip Status Line
**			method(IN)			- The method to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setMethodInReqLine _ARGS_ ((SipReqLine *line, \
		SIP_S8bit *method, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromReqLine
** Description:gets the Sip Addr Spec structure from the Sip Request Line
** Parameters:
**			line(IN) 			- Sip RequestLine
**			pReqUri(OUT)		- Addr Spec structure to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromReqLine _ARGS_ ((SipReqLine *line, \
		SipAddrSpec *pReqUri, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromReqLine
** Description:gets the Sip Addr Spec structure from the Sip Request Line
** Parameters:
**			line(IN) 			- Sip RequestLine
**			pReqUri(OUT)		- Addr Spec structure to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromReqLine _ARGS_ ((SipReqLine *line, \
		SipAddrSpec **ppReqUri, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInReqLine
** Description:sets the Sip Addr Spec structure in the Sip Request Line
** Parameters:
**			line(IN/OUT) 			- Sip RequestLine
**			pReqUri(IN)			- Addr Spec structure to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInReqLine _ARGS_ ((SipReqLine *line, \
		SipAddrSpec *req_uri, SipError *err));


/***********************************************************************
** Function: sip_getVersionFromReqLine
** Description:gets the Version fromthe Sip Request Line
** Parameters:
**			line(IN) 			- Sip RequestLine
**			version(OUT)			- Version to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getVersionFromReqLine _ARGS_ ((SipReqLine *line, \
		SIP_S8bit **version, SipError *err));


/***********************************************************************
** Function: sip_setVersionInReqLine
** Description:gets the Version in the Sip Request Line
** Parameters:
**			line(IN/OUT) 			- Sip RequestLine
**			version(IN)			- Version to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setVersionInReqLine _ARGS_ ((SipReqLine *line, \
		SIP_S8bit *version, SipError *err));


/***********************************************************************
** Function: sip_getDayOfWeekFromDateStruct
** Description:gets the Day of the Week from the Sip Date Struct
** Parameters:
**			dStruct(IN) 			- Sip Date Struct
**			day(OUT)				- The Day of the Week to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDayOfWeekFromDateStruct _ARGS_((SipDateStruct *dstruct, \
		en_DayOfWeek *day, SipError *err));


/***********************************************************************
** Function: sip_setDayOfWeekInDateStruct
** Description:sets the Day of the Week in the Sip Date Struct
** Parameters:
**			dStruct(IN/OUT) 			- Sip Date Struct
**			day(IN)					- The Day of the Week to set
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDayOfWeekInDateStruct _ARGS_((SipDateStruct *dstruct, \
		en_DayOfWeek day, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getDateFormatFromDateStruct
** Description:gets the Date Format from the Sip Date Struct
** Parameters:
**			dStruct(IN/OUT) 			- Sip Date Struct
**			pDate(OUT)				- The Date Format to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateFormatFromDateStruct _ARGS_(	(SipDateStruct *dstruct, \
		SipDateFormat *pDate, SipError *err));

#else
/***********************************************************************
** Function: sip_getDateFormatFromDateStruct
** Description:gets the Date Format from the Sip Date Struct
** Parameters:
**			dStruct(IN) 			- Sip Date Struct
**			pDate(OUT)				- The Date Format to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateFormatFromDateStruct _ARGS_(	(SipDateStruct *dstruct, \
		SipDateFormat **ppDate, SipError *err));

#endif

/***********************************************************************
** Function: sip_setDateFormatInDateStruct
** Description:sets the Date Format in the Sip Date Struct
** Parameters:
**			dStruct(IN/OUT) 			- Sip Date Struct
**			pDate(IN)				- The Date Format to set
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDateFormatInDateStruct _ARGS_((SipDateStruct *dstruct, \
		SipDateFormat *date, SipError *err));


/***********************************************************************
** Function: sip_setTimeFormatInDateStruct
** Description:sets the Time Format in the Sip Date Struct
** Parameters:
**			dStruct(IN/OUT) 			- Sip Date Struct
**			time(IN)				- The Time Format to set
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setTimeFormatInDateStruct _ARGS_((SipDateStruct *dstruct, \
		SipTimeFormat *time, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getTimeFormatFromDateStruct
** Description:gets the Time Format from the Sip Date Struct
** Parameters:
**			dStruct(IN) 			- Sip Date Struct
**			pTime(OUT)				- The Time Format to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTimeFormatFromDateStruct _ARGS_((SipDateStruct *dstruct, \
		SipTimeFormat *pTime, SipError *err));

#else
/***********************************************************************
** Function: sip_getTimeFormatFromDateStruct
** Description:gets the Time Format from the Sip Date Struct
** Parameters:
**			dStruct(IN) 			- Sip Date Struct
**			pTime(OUT)				- The Time Format to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTimeFormatFromDateStruct _ARGS_((SipDateStruct *dstruct, \
		SipTimeFormat **ppTime, SipError *err));

#endif

/***********************************************************************
** Function: sip_setOptionInSupportedHdr
** Description:sets the Options field in the Sip Supported Header
** Parameters:
**			hdr(IN/OUT) 				- Sip Supported Header
**			pOptions(IN)			- The Options field to set
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setOptionInSupportedHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pOption, SipError *pErr));


/***********************************************************************
** Function: sip_getOptionFromSupportedHdr
** Description:gets the Options field from the Sip Supported Header
** Parameters:
**			hdr(IN) 				- Sip Supported Header
**			pOptions(OUT)			- The Options field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getOptionFromSupportedHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppOption, SipError *pErr));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getDateStructFromExpires
** Description:gets the Date Structure from the Sip Expires Structure
** Parameters:
**			hdr(IN) 			- Sip Expires Structure
**			pDateStruct(OUT)	- The date structure to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateStructFromExpires _ARGS_((SipExpiresStruct *hdr, \
		SipDateStruct *date_struct, SipError *err));

#else
/***********************************************************************
** Function: sip_getDateStructFromExpires
** Description:gets the Date Structure from the Sip Expires Structure
** Parameters:
**			hdr(IN) 			- Sip Expires Structure
**			pDateStruct(OUT)	- The date structure to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDateStructFromExpires _ARGS_((SipExpiresStruct *hdr, \
		SipDateStruct **date_struct, SipError *err));

#endif

/***********************************************************************
** Function: sip_setDateStructInExpires
** Description:sets the Date Structure in the Sip Expires Structure
** Parameters:
**			hdr(IN/OUT) 			- Sip Expires Structure
**			pDateStruct(IN)		- The date structure to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDateStructInExpires _ARGS_((SipExpiresStruct *hdr, \
		SipDateStruct *date_struct, SipError *err));


/***********************************************************************
** Function: sip_getSecondsFromExpires
** Description:gets the Seconds field from the Sip Expires Struct
** Parameters:
**			hdr(IN) 			- Sip Expires Struct
**			sec(OUT)			- The seconds field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSecondsFromExpires _ARGS_((SipExpiresStruct *hdr, \
		SIP_U32bit *dSec, SipError *err));


/***********************************************************************
** Function: sip_setSecondsInExpires
** Description:sets the Seconds field in the Sip Expires Struct
** Parameters:
**			hdr(IN/OUT) 			- Sip Expires Struct
**			sec(IN)				- The seconds field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSecondsInExpires _ARGS_((SipExpiresStruct *hdr, \
		SIP_U32bit dSec, SipError *err));


/***********************************************************************
** Function: sip_getTypeFromExpires
** Description:gets the Type field from the Sip Expires Struct
** Parameters:
**			hdr(IN/OUT) 			- Sip Expires Struct
**			dType(OUT)			- The Type field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getTypeFromExpires _ARGS_((SipExpiresStruct *hdr, \
		en_ExpiresType *dType, SipError *err));


/***********************************************************************
** Function: sip_getUriFromCallInfoHdr
** Description:gets the Uri field from the Sip CallInfo Header
** Parameters:
**			hdr(IN) 				- Sip CallInfo Header
**			ppUri(OUT)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getUriFromCallInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppUri, SipError *pErr));


/***********************************************************************
** Function: sip_setUriInCallInfoHdr
** Description:sets the Uri field in the Sip CallInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CallInfo Header
**			pUri(IN)				- The Uri field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setUriInCallInfoHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pUri, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromCallInfoHdr
** Description:gets the number of Parameters from the Sip CallInfo Header
** Parameters:
**			hdr(IN) 			- Sip CallInfo Header
**			count(OUT)			- The Parameter count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromCallInfoHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromCallInfoHdr
** Description:gets the Parameters at an index from the Sip CallInfo Header
** Parameters:
**			hdr(IN) 			- Sip CallInfo Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromCallInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromCallInfoHdr
** Description:gets the Parameter at an index from the Sip CallInfo Header
** Parameters:
**			hdr(IN) 			- Sip CallInfo Header
**			ppParam(OUT)		- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromCallInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam **ppParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInCallInfoHdr
** Description:inserts the Parameter at an index in the Sip CallInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CallInfo Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInCallInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteParamAtIndexInCallInfoHdr
** Description:deletes the Parameter at an index in the Sip CallInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CallInfo Header
**			index(IN)			- The index at which the Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInCallInfoHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setParamAtIndexInCallInfoHdr
** Description:sets the Parameter at an index in the Sip CallInfo Header
** Parameters:
**			hdr(IN/OUT) 			- Sip CallInfo Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInCallInfoHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_getDispTypeFromContentDispositionHdr
** Description:gets the DispType field from the Sip ContentDisposition Header
** Parameters:
**			hdr(IN) 				- Sip ContentDisposition Header
**			ppDispType(OUT)				- The DispType field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispTypeFromContentDispositionHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppDispType, SipError *pErr));


/***********************************************************************
** Function: sip_setDispTypeInContentDispositionHdr
** Description:sets the DispType field in the Sip ContentDisposition Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ContentDisposition Header
**			pDispType(IN)				- The DispType field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispTypeInContentDispositionHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pDispType, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromContentDispositionHdr
** Description:gets the number of Parameters from the Sip ContentDisposition Header
** Parameters:
**			hdr(IN) 			- Sip ContentDisposition Header
**			count(OUT)			- The Parameter count to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
		SIP_U32bit *count, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromContentDispositionHdr
** Description:gets the Parameters at an index from the Sip ContentDisposition Header
** Parameters:
**			hdr(IN) 			- Sip ContentDisposition Header
**			pParam(OUT)			- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromContentDispositionHdr
** Description:gets the Parameter at an index from the Sip ContentDisposition Header
** Parameters:
**			hdr(IN) 			- Sip ContentDisposition Header
**			ppParam(OUT)		- The Parameter to retrieve
**			index(IN)			- The index at which the Parameter is retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam **ppParam, SIP_U32bit index, SipError *err));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInContentDispositionHdr
** Description:inserts the Parameter at an index in the Sip ContentDisposition Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ContentDisposition Header
**			pParam(IN)			- The Parameter to insert
**			index(IN)			- The index at which the Parameter is inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_deleteParamAtIndexInContentDispositionHdr
** Description:deletes the Parameter at an index in the Sip ContentDisposition Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ContentDisposition Header
**			index(IN)			- The index at which the Parameter is deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit index, SipError *err));


/***********************************************************************
** Function: sip_setParamAtIndexInContentDispositionHdr
** Description:sets the Parameter at an index in the Sip ContentDisposition Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ContentDisposition Header
**			pParam(IN)			- The Parameter to set
**			index(IN)			- The index at which the Parameter is set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInContentDispositionHdr _ARGS_ ((SipHeader *hdr, \
	SipParam *pParam, SIP_U32bit index, SipError *err));



/***********************************************************************
** Function: sip_getLangTagFromContentLanguageHdr
** Description:gets the LangTag field from the Sip ContentLanguage Header
** Parameters:
**			hdr(IN) 				- Sip ContentLanguage Header
**			ppLangTag(OUT)				- The LangTag field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getLangTagFromContentLanguageHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit **ppLangTag, SipError *pErr));


/***********************************************************************
** Function: sip_setLangTagInContentLanguageHdr
** Description:sets the LangTag field in the Sip ContentLanguage Header
** Parameters:
**			hdr(IN/OUT) 			- Sip ContentLanguage Header
**			pLangTag(IN)				- The LangTag field to retrieve
**			err(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setLangTagInContentLanguageHdr _ARGS_(( SipHeader *hdr, \
		SIP_S8bit *pLangTag, SipError *pErr));
/***********************************************************************
** Function: sip_getTokenFromRequireHdr
** Description: gets the Tokens from the Require Header
** Parameters:
**			pHdr(IN)			- Sip Require Header
**			token(OUT)			- The Token retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getTokenFromRequireHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **token, SipError *err));

/***********************************************************************
** Function: sip_setTokenInRequireHdr
** Description: sets the Tokens in the Require Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Require Header
**			token(IN)			- The Token to set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setTokenInRequireHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *token, SipError *err));
/***********************************************************************
** Function: sip_getOrganizationFromOrganizationHdr
** Description: gets the Organization from the Organization Header
** Parameters:
**			pHdr(IN)			- Sip Organization Header
**			organization(OUT)	- The organization retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getOrganizationFromOrganizationHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **organization, SipError *err));

/***********************************************************************
** Function: sip_setOrganizationInOrganizationHdr
** Description: sets the Organization in the Organization Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Organization Header
**			organization(IN)	- The organization set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setOrganizationInOrganizationHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *organization, SipError *err));

/***********************************************************************
** Function: sip_getAgentFromUserAgentHdr
** Description: gets the Agent from the Agent  Header
** Parameters:
**			pHdr(IN)			- Sip Agent Header
**			agent(OUT)			- The Agent field  retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getAgentFromUserAgentHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit **agent, SipError *err));

/***********************************************************************
** Function: sip_setAgentInUserAgentHdr
** Description: sets the Agent in the Agent  Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Agent Header
**			agent(IN)			- The Agent field  set
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setAgentInUserAgentHdr _ARGS_((SipHeader *hdr,\
		SIP_S8bit *agent, SipError *err));

/***********************************************************************
** Function:  sip_getMethodFromAllowHdr
** Description: Gets the pMethod field pValue from allow pHeader.
** Parameters:
**			hdr(IN)		- Sip Agent Header
**			pMethod(OUT)- The pMethod field  got
**			pErr(OUT)	- Possible Error value (see API ref doc)
**
**********************************************************************/
extern SipBool sip_getMethodFromAllowHdr _ARGS_((SipHeader *hdr,\
			SIP_S8bit **pMethod, SipError *err));

/***********************************************************************
** Function:  sip_setMethodInAllowHdr
** Description: Sets the pMethod field in allow pHeader.
** Parameters:
**			hdr(IN/OUT)		- Sip Agent Header
**			pMethod(IN)		- The pMethod field  set
**			pErr(OUT)		- Possible Error value (see API ref doc)
**
***********************************************************************/
extern SipBool sip_setMethodInAllowHdr _ARGS_((SipHeader *hdr,\
			SIP_S8bit *pMethod, SipError *err));

#ifdef SIP_SESSIONTIMER
/***********************************************************************
** Function: sip_getSecondsFromMinSEHdr
** Description:gets the Seconds field from the Sip MinSE Header
** Parameters:
**			hdr(IN) 			- Sip MinSE Header
**			sec(OUT)			- The seconds field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSecondsFromMinSEHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit *sec, SipError *err));

/***********************************************************************
** Function: sip_setSecondsInMinSEHdr
** Description:sets the Seconds field in the Sip MinSE Header
** Parameters:
**			hdr(IN/OUT) 			- Sip MinSE Header
**			sec(IN)				- The seconds field to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSecondsInMinSEHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit sec, SipError *err));

extern SipBool sip_getNameValuePairCountFromMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SIP_U32bit *dCount,SipError *pErr ));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_getNameValuePairAtIndexFromMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SipNameValuePair **ppNameValue,\
	SIP_U32bit index, SipError *err ));
#else
extern SipBool sip_getNameValuePairAtIndexFromMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));
#endif

extern SipBool sip_setNameValuePairAtIndexInMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_insertNameValuePairAtIndexInMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_deleteNameValuePairAtIndexInMinSEHdr _ARGS_( (\
	SipHeader *pMinSE, SIP_U32bit index, \
	SipError *err ));

/***********************************************************************
** Function: sip_getSecondsFromSessionExpiresHdr
** Description:gets the Seconds field from the Sip SessionExpires Header
** Parameters:
**			hdr(IN) 			- Sip SessionExpires Header
**			sec(OUT)			- The seconds field to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getSecondsFromSessionExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit *sec, SipError *err));

/***********************************************************************
** Function: sip_setSecondsInSessionExpiresHdr
** Description:sets the Seconds field in the Sip SessionExpires Header
** Parameters:
**			hdr(IN/OUT) 		- Sip SessionExpires Header
**			sec(IN)				- The seconds field to set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setSecondsInSessionExpiresHdr _ARGS_ ((SipHeader *hdr, \
	SIP_U32bit sec, SipError *err));

extern SipBool sip_getNameValuePairCountFromSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SIP_U32bit *dCount,SipError *pErr ));

extern SipBool sip_getRefresherFromSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SIP_S8bit **pRefresher,SipError *pErr ));

extern SipBool sip_setRefresherInSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SIP_S8bit *pRefresher,SipError *pErr ));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_getNameValuePairAtIndexFromSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SipNameValuePair **ppNameValue,\
	SIP_U32bit index, SipError *err ));
#else
extern SipBool sip_getNameValuePairAtIndexFromSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));
#endif

extern SipBool sip_setNameValuePairAtIndexInSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_insertNameValuePairAtIndexInSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SipNameValuePair *pNameValue,\
	SIP_U32bit index, SipError *err ));

extern SipBool sip_deleteNameValuePairAtIndexInSessionExpiresHdr _ARGS_( (\
	SipHeader *pSessionExpires, SIP_U32bit index, \
	SipError *err ));

#endif

/*****************************************************************
**
** FUNCTION:  sip_getTypeFromBadHdr
**
** DESCRIPTION: This function retrieves the header type from the bad
**		header. This is the type corresponding to the name of the
**		header.
**
***************************************************************/
extern SipBool sip_getTypeFromBadHdr _ARGS_((SipBadHeader *hdr,\
	en_HeaderType *pType, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_getNameFromBadHdr
**
** DESCRIPTION: This function retrieves the name from an Unknown
**		SIP header
**
***************************************************************/
extern SipBool sip_getNameFromBadHdr _ARGS_((SipBadHeader *hdr,\
	SIP_S8bit **pName, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_getBodyFromBadHdr
**
** DESCRIPTION: This function retrieves the body from an Unknown SIP
**		header
**
***************************************************************/
extern SipBool sip_getBodyFromBadHdr _ARGS_((SipBadHeader *hdr,\
	SIP_S8bit **pBody, SipError *err));

extern SipBool sip_getNameFromNameValuePair _ARGS_( (\
         SipNameValuePair *pNameValue, SIP_S8bit **pName, SipError *err));

extern SipBool sip_setNameInNameValuePair _ARGS_( (\
         SipNameValuePair *pNameValue, SIP_S8bit *pName, SipError *err));

extern SipBool sip_getValueFromNameValuePair _ARGS_( (\
         SipNameValuePair *pNameValue, SIP_S8bit **pValue, SipError *err));

extern SipBool sip_setValueInNameValuePair _ARGS_( (\
         SipNameValuePair *pNameValue, SIP_S8bit *pValue, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromReplyToHdr
**
** DESCRIPTION: This function retrieives the number of parametrs
**		from a SIP ReplyTo pHeader
**
***************************************************************/
extern SipBool sip_getParamCountFromReplyToHdr _ARGS_((SipHeader *hdr, \
			SIP_U32bit *count, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromReplyToHdr
**
** DESCRIPTION: This function retrieves a paarmeter at a specified
**		index from a SIP ReplyTo pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromReplyToHdr _ARGS_( (SipHeader *hdr,\
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));
#else
extern SipBool sip_getParamAtIndexFromReplyToHdr _ARGS_( (SipHeader *hdr,\
	 SipNameValuePair **ppParam, SIP_U32bit index, SipError *err));
#endif

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function inserts a parameter at a specified
**		in a SIp ReplyTo pHeader
**
***************************************************************/
extern SipBool sip_insertParamAtIndexInReplyToHdr _ARGS_( (SipHeader *hdr, \
		SipNameValuePair *pParam, SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function deletes a parameter at a specified
**		index in a SIP ReplyTo pHeader
**
***************************************************************/
extern SipBool sip_deleteParamAtIndexInReplyToHdr _ARGS_((SipHeader *hdr,\
			SIP_U32bit index, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInReplyToHdr
**
** DESCRIPTION: This function sets a parameter at a specified index
**		in a SIP ReplyTo pHeader
**
***************************************************************/
extern SipBool sip_setParamAtIndexInReplyToHdr _ARGS_( (SipHeader *hdr, \
			SipNameValuePair *pParam, SIP_U32bit index, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getDispNameFromReplyToHdr
**
** DESCRIPTION:This function retrieves the display-pName field from
**		a SIP Reply-To pHeader
**
***************************************************************/
extern SipBool sip_getDispNameFromReplyToHdr _ARGS_( (SipHeader *hdr, \
			SIP_S8bit **pDispName, SipError *err));

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInReplyToHdr
**
** DESCRIPTION: This function sets the display-pName field in a SIP
**		Reply-To pHeader
**
***************************************************************/
extern SipBool sip_setDispNameInReplyToHdr _ARGS_( (SipHeader *hdr,\
			SIP_S8bit *pDispName, SipError *err));


/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromReplyToHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP Reply-To pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getAddrSpecFromReplyToHdr _ARGS_( (SipHeader *hdr,\
					SipAddrSpec *pAddrSpec, SipError *err));
#else
extern SipBool sip_getAddrSpecFromReplyToHdr _ARGS_( (SipHeader *hdr,\
					SipAddrSpec **ppAddrSpec, SipError *err));
#endif

/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInReplyToHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		Reply-To pHeader
**
***************************************************************/
extern SipBool sip_setAddrSpecInReplyToHdr _ARGS_( (SipHeader *hdr, \
			SipAddrSpec *pAddrSpec, SipError *err));




#ifdef SIP_PRIVACY
/*****************************************************************
**
** FUNCTION:  sip_getDisplayNameFromPAssertIdHdr
**
** DESCRIPTION: This function retrieves the Display Name field from a
**		SIP P-Asserted-Id pHeader
**
***************************************************************/
extern SipBool sip_getDisplayNameFromPAssertIdHdr _ARGS_((SipHeader *hdr, 
			SIP_S8bit **pDisplayName, SipError *err)) ;
/*****************************************************************
**
** FUNCTION:  sip_getNameValuePairCountFromPrivacyHdr
**
** DESCRIPTION: This function retrieives the number of Privacy Values 
**		present in a SIP Privacy Header
**
**************************************************************/
extern SipBool sip_getNameValuePairCountFromPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SIP_U32bit *dCount,SipError *pErr ));


/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromPAssertIdHdr
**
** DESCRIPTION: This function retrieves the AddrSpec field from a
**		SIP PAssertId pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getAddrSpecFromPAssertIdHdr _ARGS_((SipHeader *hdr,\
		 SipAddrSpec *pAddrSpec, SipError *err)) ;
#else
extern SipBool sip_getAddrSpecFromPAssertIdHdr _ARGS_((SipHeader *hdr, \
			SipAddrSpec **ppAddrSpec, SipError *err)) ;
#endif


/*****************************************************************
**
** FUNCTION:  sip_setDisplayNameInPAssertIdHdr
**
** DESCRIPTION: This function sets the display-name field in a SIP
**		PAssertedId pHeader
**
***************************************************************/
extern SipBool sip_setDisplayNameInPAssertIdHdr _ARGS_ ( (SipHeader *hdr,\
		 SIP_S8bit *pDisplayName, SipError *err)) ;

/*****************************************************************
**
** FUNCTION:  sip_getNameValuePairAtIndexFromPrivacyHdr
**
** DESCRIPTION: This function retrieves a Privacy Value at aspecified 
** 				index in SIP Privacy header 
**		
**
***************************************************************/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_getNameValuePairAtIndexFromPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SipNameValuePair **ppNameValue,\
	SIP_U32bit dIndex, SipError *pErr ));
#else
extern SipBool sip_getNameValuePairAtIndexFromPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	SIP_U32bit dIndex, SipError *pErr ));
#endif
/*****************************************************************
**
** FUNCTION:  sip_setNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function sets a Value at a specified index in a
**		SIP Privacy pHeader
**
***************************************************************/
extern SipBool sip_setNameValuePairAtIndexInPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	SIP_U32bit dIndex, SipError *pErr ));

/*****************************************************************
**
** FUNCTION:  sip_deleteNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function deletes a Privacy value at a 
** specified index in a SIP Privacy header
**
***************************************************************/
extern SipBool sip_deleteNameValuePairAtIndexInPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SIP_U32bit dIndex, \
	SipError *pErr ));


/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInPAssertIdHdr
**
** DESCRIPTION: This function sets the pAssertId header field 
**             in a SIP PAssertId Header.
**
***************************************************************/
extern SipBool sip_setAddrSpecInPAssertIdHdr _ARGS_( (SipHeader *hdr, \
		SipAddrSpec *pAddrSpec, SipError *err)) ;
/*****************************************************************
**
** FUNCTION:  sip_insertNameValuePairAtIndexInPrivacyHdr
**
** DESCRIPTION: This function inserts a Privacy Value at a specified 
** index in a SIP Privacy Header
**
***************************************************************/
extern SipBool sip_insertNameValuePairAtIndexInPrivacyHdr _ARGS_( (\
	SipHeader *pPrivacy, SipNameValuePair *pNameValue,\
	SIP_U32bit dIndex, SipError *pErr ));

/*****************************************************************
**
** FUNCTION:  sip_getDisplayNameFromPPreferredtIdHdr
**
** DESCRIPTION: This function retrieves the Display Name field from a
**		SIP P-Preferred-Id pHeader
**
***************************************************************/
extern SipBool sip_getDisplayNameFromPPreferredIdHdr _ARGS_((SipHeader *hdr, 
			SIP_S8bit **pDisplayName, SipError *err)) ;

/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromPPreferredIdHdr
**
** DESCRIPTION: This function retrieves the AddrSpec field from a
**		SIP PPreferredIdHdr pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getAddrSpecFromPPreferredIdHdr _ARGS_((SipHeader *hdr,\
		 SipAddrSpec *pAddrSpec, SipError *err)) ;
#else
extern SipBool sip_getAddrSpecFromPPreferredIdHdr _ARGS_((SipHeader *hdr, \
			SipAddrSpec **ppAddrSpec, SipError *err)) ;
#endif


/*****************************************************************
**
** FUNCTION:  sip_setDisplayNameInPPreferredIdHdr
**
** DESCRIPTION: This function sets the display-name field in a SIP
**		PPreferredIdHdr pHeader
**
***************************************************************/
extern SipBool sip_setDisplayNameInPPreferredIdHdr _ARGS_ ( (SipHeader *hdr,\
		 SIP_S8bit *pDisplayName, SipError *err)) ;


/*****************************************************************
**
** FUNCTION:  sip_setAddrSpecInPPreferredIdHdr
**
** DESCRIPTION: This function sets the pAssertId header field 
**             in a SIP PPreferredIdHdr Header.
**
***************************************************************/
extern SipBool sip_setAddrSpecInPPreferredIdHdr _ARGS_( (SipHeader *hdr, \
		SipAddrSpec *pAddrSpec, SipError *err)) ;

#endif /* # ifdef SIP_PRIVACY */

#ifdef SIP_3GPP
/***********************************************************************
** Function: sip_getDispNameFromPathHdr
** Description:gets the Display Name from the Sip Path Header
** Parameters:
**			hdr(IN) 			- Sip Path Header
**			name(OUT)			- Display Name  to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispNameFromPathHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit **name, SipError *err));


/***********************************************************************
** Function: sip_setDispNameInPathHdr
** Description:sets the Display Name in the Sip Path Header
** Parameters:
**			hdr(IN/OUT) 		- Sip Path Header
**			name(IN)			- Display Name  to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispNameInPathHdr _ARGS_ ((SipHeader *hdr, \
	SIP_S8bit *name, SipError *err));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getAddrSpecFromPathHdr
** Description:gets the Addr Spec from the Sip Path Header
** Parameters:
**			hdr(IN) 			- Sip Path Header
**			pAddrSpec(OUT)		- Addr Spec to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPathHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec *pAddrSpec, SipError *err));

#else
/***********************************************************************
** Function: sip_getAddrSpecFromPathHdr
** Description:gets the Addr Spec from the Sip Path Header
** Parameters:
**			hdr(IN) 			- Sip Path Header
**			pAddrSpec(OUT)		- Addr Spec to be retrieved
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getAddrSpecFromPathHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec **ppAddrSpec, SipError *err));

#endif

/***********************************************************************
** Function: sip_setAddrSpecInPathHdr
** Description:sets the Addr Spec in the Sip Path Header
** Parameters:
**			hdr(IN/OUT) 			- Sip Path Header
**			pAddrSpec(IN)		- Addr Spec to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setAddrSpecInPathHdr _ARGS_ ((SipHeader *hdr, \
	SipAddrSpec *addr_spec, SipError *err));


/***********************************************************************
** Function:  sip_getParamCountFromPathHdr
** Description:Retrieve the number of parameters in Path header
** Parameters:
**			pHdr(IN) 			- Sip Path Header
**			pCount(OUT)			- count of params
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromPathHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit *pCount, SipError *pErr));

/***********************************************************************
** Function:  sip_getParamAtIndexFromPathHdr
** Description: Retrieves the Param at a specified in a Path header
** Parameters:
**			pHdr(IN) 			- Sip Path Header
**
**			pParam/ppParam (OUT)		_ The retrieved param
**			dIndex(IN)			- index of the param to retrieve
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
#ifndef SIP_BY_REFERENCE
extern SipBool sip_getParamAtIndexFromPathHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
extern SipBool sip_getParamAtIndexFromPathHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/***********************************************************************
** Function:  sip_setParamAtIndexInPathHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Path Header
**
**			pParam (IN)			_ The param to be set
**			dIndex(IN)			- index at which the param is to be set
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPathHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInPathHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Path Header
**
**			pParam (IN)			_ The param to be inserted
**			dIndex(IN)			- index at which the param is to be inserted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPathHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInPathHdr
** Description:
** Parameters:
**			pHdr(IN/OUT) 			- Sip Path Header
**			dIndex(IN)			- index at which the param is to be deleted
**			err(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPathHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit dIndex, SipError *pErr));






/***********************************************************************
 ** Function: sip_getAccessTypeFromPanInfoHdr
 ** Description:This function retrieves the pAccessType field from
 **             a SIP P-Access-Network-Info pHeader
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Access-Network-Info Header
 **          ppAccessType     -The field to retrieve
 **          pErr            -Possible error value
 **
 *********************************************************************/
 
extern SipBool sip_getAccessTypeFromPanInfoHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppAccessType, SipError *pErr));
         
/***********************************************************************
 ** Function: sip_setAccessTypeFromPanInfoHdr
 ** Description:This function sets the pAccessType field from
 **             a SIP P-Access-Network-Info pHeader
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Access-Network-Info Header
 **          pAccessType     -The field to set
 **          pErr            -Possible error value
 **
 *********************************************************************/
extern SipBool sip_setAccessTypeInPanInfoHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pAccessType, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromPanInfoHdr
** Description:gets the number of Parameters from the Sip P-Access-Network-Info ** Header
** Parameters:
**			pHdr(IN) 			- Sip P-Access-Network-Info Header
**			pCount(OUT)			- The Parameter count to retrieve
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamCountFromPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromPanInfoHdr
** Description:gets the Parameters at an index from the Sip PanInfo Header
** Parameters:
**			pHdr(IN) 			- Sip P-Access-Network-Info Header
**			pParam(OUT)			- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromPanInfoHdr
** Description:gets the Parameter at an index from the Sip PanInfo Header
** Parameters:
**			pHdr(IN) 			- Sip P-Access-Network-Info Header
**			ppParam(OUT)		- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair **ppParam, SIP_U32bit dIndex, SipError *pErr));

#endif

/***********************************************************************
** Function: sip_setParamAtIndexInPanInfoHdr
** Description:sets the Parameter at an index in the Sip PanInfo Header
** Parameters:
**			pHdr(IN/OUT)		- Sip P-Access-Network-Info Header
**			pParam(IN)			- The Parameter to set
**			dIndex(IN)			- The index at which the Parameter is set
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_insertParamAtIndexInPanInfoHdr
** Description:inserts the Parameter at an index in the Sip PanInfo Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip P-Access-Network-Info Header
**			pParam(IN)			- The Parameter to insert
**			dIndex(IN)			- The index at which the Parameter is inserted
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_deleteParamAtIndexInPanInfoHdr
** Description:deletes the Parameter at an index in the Sip PanInfo Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip P-Access-Network-Info Header
**			dIndex(IN)			- The index at which the Parameter is deleted
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPanInfoHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit dIndex, SipError *pErr));




/***********************************************************************
 ** Function: sip_getAccessTypeFromPcVectorHdr
 ** Description:This function retrieves the pAccessType field from
 **             a SIP P-Charging-Vector pHeader
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Charging-Vector Header
 **          pAccessType     -The field to retrieve
 **          pErr            -Possible error value
 **
 *********************************************************************/
 
extern SipBool sip_getAccessTypeFromPcVectorHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppAccessType, SipError *pErr));
         
/***********************************************************************
 ** Function: sip_setAccessTypeFromPcVectorHdr
 ** Description:This function sets the pAccessType field from
 **             a SIP PcVector Header
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Charging-Vector Header
 **          pAccessType     -The field to set
 **          pErr            -Possible error value
 **
 *********************************************************************/
extern SipBool sip_setAccessTypeInPcVectorHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pAccessType, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromPcVectorHdr
** Description: Gets the number of Parameters from the Sip PcVector
**              Header
** Parameters:
**			pHdr(IN) 			- Sip P-Charging-Vector Header
**			pCount(OUT)			- The Parameter count to retrieve
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamCountFromPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromPcVectorHdr
** Description:gets the Parameters at an index from the Sip PcVector Header
** Parameters:
**			pHdr(IN) 			- Sip P-Charging-Vector Header
**			pParam(OUT)			- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromPcVectorHdr
** Description:gets the Parameter at an index from the Sip PcVector Header
** Parameters:
**			pHdr(IN) 			- Sip P-Charging-Vector Header
**			ppParam(OUT)		- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair **ppParam, SIP_U32bit dIndex, SipError *pErr));

#endif

/***********************************************************************
** Function: sip_setParamAtIndexInPcVectorHdr
** Description:sets the Parameter at an index in the Sip PcVector Header
** Parameters:
**			pHdr(IN/OUT)		- Sip P-Charging-Vector Header
**			pParam(IN)			- The Parameter to set
**			dIndex(IN)			- The index at which the Parameter is set
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function: sip_insertParamAtIndexInPcVectorHdr
** Description:inserts the Parameter at an index in the Sip PcVector Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip P-Charging-Vector Header
**			pParam(IN)			- The Parameter to insert
**			dIndex(IN)			- The index at which the Parameter is inserted
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
	SipNameValuePair *pParam, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_deleteParamAtIndexInPcVectorHdr
** Description:deletes the Parameter at an index in the Sip PcVector Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip P-Charging-Vector Header
**			dIndex(IN)			- The index at which the Parameter is deleted
**			pErr(OUT)			- Possible error value
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPcVectorHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
 ** Function: sip_getAccessValueFromPcVectorHdr
 ** Description:This function retrieves the pAccessValue field from
 **             a SIP P-Charging-Vector pHeader
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Charging-Vector Header
 **          pAccessValue    -The field to retrieve
 **          pErr            -Possible error value
 **
 *********************************************************************/
 
extern SipBool sip_getAccessValueFromPcVectorHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppAccessValue, SipError *pErr));
         
/***********************************************************************
 ** Function: sip_setAccessTypeFromPcVectorHdr
 ** Description:This function sets the pAccessValue field from
 **             a SIP PcVector Header
 ** Parameters:
 **          pHdr(IN/OUT)    -SIP P-Charging-Vector Header
 **          pAccessValue     -The field to set
 **          pErr            -Possible error value
 **
 *********************************************************************/
extern SipBool sip_setAccessValueInPcVectorHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pAccessValue, SipError *pErr));


/*************************************************************
**
** FUNCTION:  sip_getDispNameFromServiceRouteHdr
**
** DESCRIPTION:This function retrieves the Display Name field from
**		a SIP ServiceRoute pHeader
**
**************************************************************/

SipBool sip_getDispNameFromServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SIP_S8bit **pDispName, SipError *pErr)) ;

/*****************************************************************
**
** FUNCTION:  sip_setDispNameInServiceRouteHdr
**
** DESCRIPTION: This function sets the Display-Name field in a SIP
**		Service-Route pHeader
**
***************************************************************/
SipBool sip_setDispNameInServiceRouteHdr _ARGS_(
	(SipHeader *pHdr, SIP_S8bit *pDispName, SipError *pErr)) ;


/*****************************************************************
**
** FUNCTION:  sip_getAddrSpecFromServiceRouteHdr
**
** DESCRIPTION: This function retrieves the dAddr-spec field from a
**		SIP ServiceRoute pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
SipBool sip_getAddrSpecFromServiceRouteHdr _ARGS_(
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)) ;
#else
SipBool sip_getAddrSpecFromServiceRouteHdr _ARGS_(
	(SipHeader *pHdr, SipAddrSpec **ppAddrSpec, SipError *pErr)) ;
#endif

/************************************************************
**
** FUNCTION:  sip_setAddrSpecInServiceRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		ServiceRoute Header
**
***************************************************************/
SipBool sip_setAddrSpecInServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SipAddrSpec *pAddrSpec, SipError *pErr)); 

/*****************************************************************
**
** FUNCTION:  sip_getParamCountFromServiceRouteHdr
**
** DESCRIPTION: This function sets the dAddr-spec field in a SIP
**		ServiceRoute pHeader
**
***************************************************************/

SipBool sip_getParamCountFromServiceRouteHdr _ARGS_(
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr)) ;

/*****************************************************************
**
** FUNCTION:  sip_getParamAtIndexFromServiceRouteHdr
**
** DESCRIPTION: This function gets the param field at a given index
**				 in a SIP Service Route pHeader
**
***************************************************************/
#ifndef SIP_BY_REFERENCE
SipBool sip_getParamAtIndexFromServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));
#else
SipBool sip_getParamAtIndexFromServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));
#endif

/*****************************************************************
**
** FUNCTION:  sip_setParamAtIndexInServiceRouteHdr
**
** DESCRIPTION: This function sets a param at a specified index
**		in a ServiceRoute Header
**
***************************************************************/

SipBool sip_setParamAtIndexInServiceRouteHdr _ARGS_(
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)) ;

/*****************************************************************
**
** FUNCTION:  sip_insertParamAtIndexInServiceRouteHdr
**
** DESCRIPTION: This function inserts a param at a specified index
**		in a ServiceRoute Header
**
***************************************************************/
SipBool sip_insertParamAtIndexInServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr)) ;

/*****************************************************************
**
** FUNCTION:  sip_deleteParamAtIndexInPathHdr
**
** DESCRIPTION: This function deletes a param at a specified index
**		in a Path Header
**
***************************************************************/
SipBool sip_deleteParamAtIndexInServiceRouteHdr _ARGS_ (
	(SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)) ;


#endif /*3GPP */

/***********************************************************************
** Function: sip_getDispTypeFromReasonHdr
** Description:gets the DispType field from the Sip Reason Header
** Parameters:
**			pHdr(IN) 				- Sip Reason Header
**			ppDispType(OUT)			- The DispType field to retrieve
**			pErr(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getDispTypeFromReasonHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit **ppDispType, SipError *pErr));


/***********************************************************************
** Function: sip_setDispTypeInReasonHdr
** Description:sets the DispType field in the Sip Reason Header
** Parameters:
**			pHdr(IN/OUT) 			- Sip Reason Header
**			pDispType(IN)			- The DispType field to retrieve
**			pErr(OUT)				- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setDispTypeInReasonHdr _ARGS_(( SipHeader *pHdr, \
		SIP_S8bit *pDispType, SipError *pErr));

/***********************************************************************
** Function: sip_getParamCountFromReasonHdr
** Description:gets the number of Parameters from the Sip Reason Header
** Parameters:
**			pHdr(IN) 			- Sip Reason Header
**			pCount(OUT)			- The Parameter count to retrieve
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromReasonHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr));


#ifndef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromReasonHdr
** Description:gets the Parameters at an index from the Sip Reason Header
** Parameters:
**			pHdr(IN) 			- Sip Reason Header
**			pParam(OUT)			- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromReasonHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromReasonHdr
** Description:gets the Parameter at an index from the Sip Reason Header
** Parameters:
**			pHdr(IN) 			- Sip Reason Header
**			ppParam(OUT)		- The Parameter to retrieve
**			dIndex(IN)			- The index at which the Parameter is retrieved
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromReasonHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#endif

/***********************************************************************
** Function: sip_insertParamAtIndexInReasonHdr
** Description:inserts the Parameter at an index in the Sip Reason Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Reason Header
**			pParam(IN)			- The Parameter to insert
**			dIndex(IN)			- The index at which the Parameter is inserted
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInReasonHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_deleteParamAtIndexInReasonHdr
** Description:deletes the Parameter at an index in the Sip Reason Header
** Parameters:
**			pHdr(IN/OUT) 		- Sip Reason Header
**			dIndex(IN)			- The index at which the Parameter is deleted
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInReasonHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit dIndex, SipError *pErr));


/***********************************************************************
** Function: sip_setParamAtIndexInReasonHdr
** Description:sets the Parameter at an index in the Sip Reason Header
** Parameters:
**			pHdr(IN/OUT)		- Sip Reason Header
**			pParam(IN)			- The Parameter to set
**			dIndex(IN)			- The index at which the Parameter is set
**			pErr(OUT)			- Possible error value(see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInReasonHdr _ARGS_ ((SipHeader *pHdr, \
	SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#ifdef SIP_3GPP
/* PcfAddr header */
/***********************************************************************
** Function:  sip_getParamCountFromPcfAddrHdr
** Description: This function retrieves the number of params in a PcfAddr
**		header
** Parameters:
**			pHdr(IN)			- Sip PcfAddr Header
**			pCount(OUT)			- The number of parameters
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamCountFromPcfAddrHdr _ARGS_ ((SipHeader *pHdr,SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function:  sip_getParamAtIndexFromPcfAddrHdr
** Description: This function retrieves a param at a specified index in
		PcfAddr header
** Parameters:
**			pHdr(IN)			- Sip PcfAddrHeader
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPcfAddrHdr _ARGS_((SipHeader *pHdr, SipParam **ppParam, SIP_U32bit dIndex, SipError *pErr));

#else
/***********************************************************************
** Function:  sip_getParamAtIndexFromPcfAddrHdr
** Description: This function retrieves a param at a specified index in
		PcfAddr header
** Parameters:
**			pHdr(IN)			- Sip PcfAddr Header
**			pParam/ppParam(OUT)		- The retrieved param
**			dIndex (IN)				_ The index of the param to be retrieved
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_getParamAtIndexFromPcfAddrHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

#endif
/***********************************************************************
** Function:  sip_setParamAtIndexInPcfAddrHdr
** Description: This function sets a param at a specified index in
		PcfAddr header
** Parameters:
**			pHdr(IN)			- Sip PcfAddr Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_setParamAtIndexInPcfAddrHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_insertParamAtIndexInPcfAddrHdr
** Description: This function insert a param at a specified index in
		PcfAddr header
** Parameters:
**			pHdr(IN)			- Sip PcfAddr Header
**			pParam/ppParam(IN)		- The param to be set
**			dIndex (IN)				_ The index of the param 
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_insertParamAtIndexInPcfAddrHdr _ARGS_((SipHeader *pHdr, SipParam *pParam, SIP_U32bit dIndex, SipError *pErr));

/***********************************************************************
** Function:  sip_deleteParamAtIndexInPcfAddrHdr
** Description: This function deletes a param at a specified index in
		PcfAddr header
** Parameters:
**			pHdr(IN/OUT)			- Sip PcfAddr Header
**			pParam(IN)			- The param to be deleted
**			dIndex(OUT)			_ The index at which the param is to be deleted
**			pErr(OUT)			- Possible Error value (see API ref doc)
**
************************************************************************/
extern SipBool sip_deleteParamAtIndexInPcfAddrHdr _ARGS_((SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr));

#endif /* #ifdef SIP_3GPP */

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __SIP_GENERAL_H_ */
