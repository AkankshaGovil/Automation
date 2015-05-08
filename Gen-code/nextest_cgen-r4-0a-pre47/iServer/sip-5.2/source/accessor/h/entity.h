/**************************************************************************
** FUNCTION:
**	This file contains the prototypes of the entity header accessor APIs.
**
***************************************************************************
**
** FILENAME:
**	entcsb.h
**
** DESCRIPTION
**
**
**  DATE           NAME	                      REFERENCE
**  ---			  ------					-------------
** 17Nov99	    B.Borthakur     		       	Original   
**
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/

#ifndef _SIP_ENTITY_H_	
#define _SIP_ENTITY_H_	

#include "sipcommon.h"
#include "sipstruct.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/***********************************************************************
** Function: sip_getLengthFromContentLengthHdr 
** Description: get Length from Content Length Header
** Parameters:	
**				hdr(IN)		- Sip Content Length Header
**				length(OUT)	- length to retrieve 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getLengthFromContentLengthHdr _ARGS_((SipHeader * hdr,  \
		SIP_U32bit * length, SipError * err));

/***********************************************************************
** Function: sip_setLengthInContentLengthHdr 
** Description: set Length in Content Length Header
** Parameters:	
**				hdr(IN/OUT)	- Sip Content Length Header
**				length(IN)	- length to set 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setLengthInContentLengthHdr _ARGS_((SipHeader * hdr,  \
		SIP_U32bit length, SipError * err));

/***********************************************************************
** Function: sip_getMediaTypeFromContentTypeHdr 
** Description: get Media from Content Type Header
** Parameters:	
**				hdr(IN)		- Sip Content Type Header
**				type(OUT)	- Content Type retrieved 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getMediaTypeFromContentTypeHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit ** type, SipError * err));

/***********************************************************************
** Function: sip_setMediaTypeInContentTypeHdr 
** Description: set Media in Content Type Header
** Parameters:	
**				hdr(IN/OUT)	- Sip Content Type Header
**				type(IN)	- Content Type to set 
**				err(OUT)  	- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setMediaTypeInContentTypeHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit * type, SipError * err));

/***********************************************************************
** Function: sip_getEncodingFromContentEncodingHdr 
** Description: get Encoding from Content Encoding Header
** Parameters:	
**				hdr(IN)			- Sip Content Encoding Header
**				encoding(OUT)	- Content Encoding retrieved 
**				err(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getEncodingFromContentEncodingHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit ** encoding, SipError * err));

/***********************************************************************
** Function: sip_setEncodingInContentEncodingHdr 
** Description: set Encoding in Content Encoding Header
** Parameters:	
**				hdr(IN/OUT)		- Sip Content Encoding Header
**				encoding(IN)	- Content Encoding to set 
**				err(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setEncodingInContentEncodingHdr _ARGS_((SipHeader * hdr,  \
		SIP_S8bit * encoding, SipError * err));

/***********************************************************************
** Function: sip_getParamCountFromContentTypeHdr 
** Description: get count of parameters from Content Type header
** Parameters:
**				pHdr(IN)	- Sip Content Type Header
**				pCount(IN)		- Count of parameters retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getParamCountFromContentTypeHdr _ARGS_(( SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr ));

#ifdef SIP_BY_REFERENCE
/***********************************************************************
** Function: sip_getParamAtIndexFromContentTypeHdr 
** Description: get parameter at index from Content Type header
** Parameters:
**				pHdr(IN)		- Sip Content Type Header
**				pParam(OUT)     - Parameter retrieved
**				index(IN)		- index at which parameter is to be retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getParamAtIndexFromContentTypeHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam **pParam, SIP_U32bit	index, SipError *pErr ));

#else
/***********************************************************************
** Function: sip_getParamAtIndexFromContentTypeHdr 
** Description: get parameter at index from Content Type header
** Parameters:
**				pHdr(IN)		- Sip Content Type Header
**				pParam(OUT)     - Parameter retrieved
**				index(IN)		- index at which parameter is to be retrieved 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_getParamAtIndexFromContentTypeHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam *pParam, SIP_U32bit	index, SipError *pErr ));

#endif
/***********************************************************************
** Function: sip_setParamAtIndexInContentTypeHdr 
** Description: set parameter at index in Content Type header
** Parameters:
**				pHdr(IN/OUT)	- Sip Content Type Header
**				pParam(IN)      - Parameter to set
**				index(IN)		- index at which parameter is to be set 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_setParamAtIndexInContentTypeHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam *pParam, SIP_U32bit index, SipError *pErr ));

/***********************************************************************
** Function: sip_insertParamAtIndexInContentTypeHdr 
** Description: insert parameter at index in Content Type header
** Parameters:
**				pHdr(IN/OUT)	- Sip Content Type Header
**				pParam(IN)      - Parameter to insert
**				index(IN)		- index at which parameter is to be inserted 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_insertParamAtIndexInContentTypeHdr _ARGS_( ( SipHeader *pHdr, \
		SipParam	*pParam, SIP_U32bit index, SipError *pErr ));

/***********************************************************************
** Function: sip_deleteParamAtIndexInContentTypeHdr 
** Description: delete parameter at index in Content Type header
** Parameters:
**				pHdr(IN/OUT)	- Sip Content Type Header
**				index(IN)		- index at which parameter is to be deleted 
**				pErr(OUT)  		- Possible error value (See API ref doc)
************************************************************************/
extern SipBool sip_deleteParamAtIndexInContentTypeHdr _ARGS_( ( SipHeader *pHdr, \
		SIP_U32bit index, SipError *pErr ));




/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
