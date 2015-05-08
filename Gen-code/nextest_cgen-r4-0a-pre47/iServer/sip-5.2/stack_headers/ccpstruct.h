/******************************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Caller & Callee preferences Related 
**              Structures

 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccpstruct.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME		REFERENCE	REASON
 ** ----	----		--------	------
 ** 8/2/2000	S.Luthra	Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef __CCPSTRUCT_H__
#define __CCPSTRUCT_H__

#include "sipstruct.h"
#include "siplist.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef SIP_CCP_VERSION10
typedef enum
{
	SipAccContactTypeFeature,
	SipAccContactTypeGeneric,
	SipAccContactTypeOther,
	SipAccContactTypeAny
}en_AcceptContactType;


typedef enum
{
	SipRejContactTypeFeature=10,
	SipRejContactTypeGeneric,
	SipRejContactTypeOther,
	SipRejContactTypeAny
}en_RejectContactType;


typedef struct
{
	en_AcceptContactType dType;
	union
	{
		SipParam	*pParam;/* feature and generic param */
		SIP_S8bit 	*pToken;/* req and explicit param */
	}u;
	SIP_RefCount	dRefCount;
}SipAcceptContactParam;


typedef struct
{
	en_RejectContactType dType;
	union
	{
		SipParam	*pParam;/* feature and generic param */
		SIP_S8bit 	*pToken;/* req and explicit param */
	}u;
	SIP_RefCount	dRefCount;
}SipRejectContactParam;


typedef struct 
{
	SipList 	slAcceptContactParams;
	SIP_RefCount	dRefCount;
}SipAcceptContactHeader;


typedef struct {
	SipList slRejectContactParams; /* of SipRejectContactParam */
	SIP_RefCount	dRefCount;
} SipRejectContactHeader;

typedef struct {
	SIP_S8bit	*pFeature;
	SIP_RefCount	dRefCount;
} SipRequestDispositionHeader;


#else
typedef enum
{
	SipAccContactTypeExt=0,
	SipAccContactTypeQvalue,
	SipAccContactTypeAny
}en_AcceptContactType;


typedef enum
{
	SipRejContactTypeExt=0,
	SipRejContactTypeQvalue,
	SipRejContactTypeAny
}en_RejectContactType;


typedef struct
{
	en_AcceptContactType dType;
	union
	{
		SipParam	*pExtParam;
		SIP_S8bit 	*pQvalue;
	}u;
	SIP_RefCount	dRefCount;
}SipAcceptContactParam;


typedef struct
{
	en_RejectContactType dType;
	union
	{
		SipParam	*pExtParam;
		SIP_S8bit 	*pQvalue;
	}u;
	SIP_RefCount	dRefCount;
}SipRejectContactParam;


typedef struct 
{
	SipAddrSpec 	*pAddrSpec;
	SIP_S8bit 	*pDispName;
	SipList 	slAcceptContactParams;
	SIP_RefCount	dRefCount;
}SipAcceptContactHeader;


typedef struct {
	SIP_S8bit *pDispName;
	SipAddrSpec *pAddrSpec;
	SipList slRejectContactParams; /* of SipRejectContactParam */
	SIP_RefCount	dRefCount;
} SipRejectContactHeader;

typedef struct {
	SIP_S8bit	*pFeature;
	SIP_RefCount	dRefCount;
} SipRequestDispositionHeader;

#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
