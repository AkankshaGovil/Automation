/******************************************************************************
 ** FUNCTION:
 **	 This file contains the protypes of the functions used to manipulate  
 **	 SipList structures. It also contains the SipList structure definitions
 ******************************************************************************
 **
 ** FILENAME:
 ** siplist.h
 **
 ** DESCRIPTION:
 **	All modules using SipLists must include this file for the structure definitions
 ** 	and the function prototypes.	
 **	
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 16/11/99		   Binu K S     		--			Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 *****************************************************************************/


#ifndef __SIPLIST__
#define __SIPLIST__

#include "sipcommon.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


/* mallocs in the .c file to be replaced with porting layer functions later */

typedef struct _SipListElement
{
 SIP_Pvoid pData;
 struct _SipListElement *next;
} SipListElement;

typedef struct
{
 SIP_U32bit size;
 SipListElement *head;
 SipListElement *tail;
 void  (*freefunc)(SIP_Pvoid );
} SipList;

typedef void (*sip_listFuncPtr)(SIP_Pvoid );
typedef void (*sip_listFuncPtrWithData)(SIP_Pvoid,SIP_Pvoid);
extern SipBool sip_listInit _ARGS_((SipList *list, \
	sip_listFuncPtr freefunc,SipError *error));
extern SipBool sip_listAppend _ARGS_((SipList *list, SIP_Pvoid data, \
	SipError *error));
extern SipBool sip_listPrepend _ARGS_((SipList *list, SIP_Pvoid data, \
	SipError *error));
extern SipBool sip_listGetAt _ARGS_((SipList *list, SIP_U32bit position, \
	SIP_Pvoid *data, SipError *error));
extern SipBool sip_listSetAt _ARGS_((SipList *list, SIP_U32bit position, \
	SIP_Pvoid data, SipError *error));
extern SipBool sip_listSizeOf _ARGS_((SipList *list, SIP_U32bit *size, \
	SipError *error));
extern SipBool sip_listInsertAt _ARGS_((SipList *list,  \
	SIP_U32bit position, SIP_Pvoid data, SipError *error));
extern SipBool sip_listDeleteAt _ARGS_((SipList *list,  \
	SIP_U32bit position, SipError *error));
extern SipBool sip_listForEach _ARGS_((SipList *list, \
	sip_listFuncPtr func, SipError *error));
extern SipBool sip_listDeleteAll _ARGS_((SipList *list,  \
	SipError *error));
extern SipBool sip_listForEachWithData _ARGS_((SipList *list,\
	sip_listFuncPtrWithData func, SIP_Pvoid *pData, SipError *error));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
