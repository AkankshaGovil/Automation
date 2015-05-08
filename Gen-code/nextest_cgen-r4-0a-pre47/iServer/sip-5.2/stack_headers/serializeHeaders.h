/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of the functions handling the 
**	serialization/deserialization of SipHeader
**
***************************************************************************
**
** FILENAME:
**  serializeHeaders.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** 16May2002  	R.Kamath			 			Initial
**
**
** Copyright 2002, Hughes Software Systems, Ltd.
***************************************************************************/
#ifndef __SIPSERIALIZEHDRS_H__
#define __SIPSERIALIZEHDRS_H__
	
#include <serialize.h>
#include "sipstruct.h"

#define SIPHEADER_ID		9999
#define HEADERTYPEENUMID	9998

typedef SIP_U32bit (*tyDeSerFn)(SIP_Pvoid,SIP_S8bit *,SIP_U32bit posn,\
					SipError *pError);

/*****************************************************************************
**** FUNCTION:	sip_initSerializationArray
****
****
**** DESCRIPTION:This API is the function that needs to be invoked before the
****			 serialization/deserialization of headers is attempted.
****
**** Parameters:
****			pError(OUT): Error Variable
****
**** Return Values:
****			SipSuccess
****			SipFail
****
***************************************************************************/
extern SipBool sip_initSerializationArray _ARGS_ ((SipError *pError));


/*****************************************************************************
**** FUNCTION:	sip_serializeSipHeader
****
****
**** DESCRIPTION:This API performs the serialization of the SipHeader
****			 that is passed to the function. 
****
**** Parameters:
****			pSipHeader(IN): SipHeader that needs to be serialized
****			pBuffer(IN/OUT):The API writes the serialized buffer to this 
****			position(IN)  :The location within the pBuffer from which the
****							API starts writing
****			pError(OUT)	  :Error Variable	
**** Return Values:
****			SIP_U32bit: This is the no. of bytes that have been written 
****			to the pBuffer. Basically the size of the serialized data
****
***************************************************************************/
SIP_U32bit	sip_serializeSipHeader(SipHeader *pSipHeader, SIP_S8bit	*pBuffer,\
									SIP_U32bit	position,SipError *pError);

/*****************************************************************************
**** FUNCTION:	sip_deserializeSipHeader
****
****
**** DESCRIPTION:This API performs the deserialization of the pBuffer
****			 that is passed to the function. 
****
**** Parameters:
****			ppSipHeader(IN/OUT): The serialized SipHeader
****			pBuffer(IN/OUT):The API reads the serialized buffer from this
****			position(IN)  :The location within the pBuffer from which the
****							API starts reading
****			pError(OUT)	  :Error Variable	
**** Return Values:
****			SIP_U32bit: This is the no. of bytes that have been serialized
****			from the pBuffer enroute to the conversion to SipHeader
****
***************************************************************************/
SIP_U32bit	sip_deserializeSipHeader(SipHeader **ppSipHeader, 
									SIP_S8bit	*pBuffer, SIP_U32bit position,\
									SipError *pError);
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif
