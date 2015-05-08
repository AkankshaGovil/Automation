/******************************************************************************
** FUNCTION:
** This header file contains the prototypes dcs related things in the 
** sipdecode.c  
**
*******************************************************************************
**
** FILENAME:
** 	dcsdecode.h
**
** DESCRIPTION:
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
**
** DATE 	   	 NAME           REFERENCE      REASON
** ----    		 ----           ---------      ------
** 4Dec2000	  T.Seshashayi		Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/


#ifndef _DCSDECODE_H_
#define _DCSDECODE_H_

#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


SipBool sip_dcs_getTypeFromName_astart _ARGS_ ((SIP_S8bit *pName,\
			en_HeaderType *pType,SipError *pError));
SipBool sip_dcs_getTypeFromName_dstart _ARGS_ ((SIP_S8bit *pName,\
			en_HeaderType *pType,SipError *pError));
SipBool sip_dcs_getTypeFromName_pstart _ARGS_ ((SIP_S8bit *pName,\
			en_HeaderType *pType,SipError *pError));
SipBool sip_dcs_getTypeFromName_rstart _ARGS_ ((SIP_S8bit *pName,\
			en_HeaderType *pType,SipError *pError));
SipBool sip_dcs_getTypeFromName_sstart _ARGS_ ((SIP_S8bit *pName,\
			en_HeaderType *pType,SipError *pError));


void sip_dcs_glbSipParserRemoveExistingHeaders _ARGS_ ((SipUnknownHeader *yyhdr,\
		SipMessage *glbSipParserSipMessage,SipError glbSipParserErrorValue));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif


#endif
