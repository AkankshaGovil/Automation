/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes dcs related things in the 
**  formmessage.c  
**
*******************************************************************************
**
** FILENAME:
** 	dcsformmessage.h
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

#ifndef _DCSFORMMESSAGE_H_
#define _DCSFORMMESSAGE_H_

#include "sipstruct.h"
#include "sipcommon.h"

#ifdef __cplusplus
extern "C" {
#endif


extern SipBool sip_dcs_formSingleGeneralHeader _ARGS_((SIP_S8bit* pEndBuff,en_HeaderType dType,\
		SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
		SipGeneralHeader *g, SIP_S8bit **out, SipError *err));


extern SipBool sip_dcs_formSingleRequestHeader _ARGS_((SIP_S8bit* pEndBuff,en_HeaderType dType,\
	SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
	SipReqHeader *s, SIP_S8bit **out, SipError *err));


extern SipBool sip_dcs_formSingleResponseHeader _ARGS_((SIP_S8bit* pEndBuff,en_HeaderType dType,\
	SIP_U32bit ndx,en_AdditionMode mode,en_HeaderForm form,\
	SipRespHeader *s, SIP_S8bit **out, SipError *err));


SipBool sip_formSipDcsAcctEntryList _ARGS_((SIP_S8bit* pEndBuff,SIP_S8bit **out,\
	SipList *list, SIP_S8bit *separator,SIP_U8bit leadingsep,\
	SipError 	*err)); 

/*
SipBool sip_formDcsRpiAuthList _ARGS_((SIP_S8bit *out, 	SipList *list,\
		SIP_S8bit *separator,SIP_U8bit leadingsep,\
		SipError 	*err));
*/


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
