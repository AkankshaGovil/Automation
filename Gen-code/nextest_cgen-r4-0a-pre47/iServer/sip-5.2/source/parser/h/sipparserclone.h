/* 
** THIS FILE IS INTERNALLY USED BY THE STACK
** THE USER SHOULD NOT INCLUDE THIS FILE DIRECTLY
**/

/**************************************************************************
** FUNCTION:
**  This file contains the INTERNAL prototypes of parser clone functions
**
***************************************************************************
**
** FILENAME:
**  sipparserclone.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** 17Nov99  	KSBinu, Arjun RC			 Initial
**
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/

#ifndef __SIP_PARSER_CLONE_H_
#define __SIP_PARSER_CLONE_H_

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern SipBool __sipParser_cloneSipParam _ARGS_((SipParam *pDest, \
	SipParam *pSource, SipError *pErr));
extern SipBool __sipParser_cloneSipContentTypeHeader _ARGS_((SipContentTypeHeader *dest, \
	SipContentTypeHeader *source, SipError *err));
extern SipBool __sipParser_bcpt_cloneSipMimeHeader _ARGS_((SipMimeHeader *pDest, \
	SipMimeHeader *pSource, SipError *pErr));
extern SipBool __validateSipAddrSpecType _ARGS_((en_AddrType dType, SipError *err));
extern SipBool __sipParser_cloneSipUrl _ARGS_((SipUrl *to,SipUrl *from, SipError *err));

extern SipBool __sipParser_cloneSipAddrSpec _ARGS_((SipAddrSpec *dest, \
	SipAddrSpec *source, SipError *err));
extern SipBool __sipParser_cloneSipStringList _ARGS_((SipList *dest, \
	SipList *source, SipError *err));
extern SipBool __sipParser_cloneSipParamList _ARGS_((SipList *dest, \
	SipList *source, SipError *err));
extern SipBool __sipParser_cloneSipFromHeader _ARGS_((SipFromHeader *dest, \
	SipFromHeader *source, SipError *err));
extern SipBool __sipParser_cloneSipToHeader _ARGS_((SipToHeader *dest, \
	SipToHeader *source, SipError *err));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
