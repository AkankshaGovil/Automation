#ifdef SIP_MWI
/************************************************************
** FUNCTION:
**	This file contains the prototypes of the
** Mesg Waiting Message Body accessor APIs.
**
*************************************************************
**
** FILENAME:
**	mesgsummary.h
**
** DESCRIPTION
**
**  DATE           NAME                    REFERENCE
**  ------        ------                  -----------
** 09Jan2002	Sasidhar P V k		   Initial Version
**
** Copyright 2002, Hughes Software Systems, Ltd.
**************************************************************/

#ifndef _SIP_MESGSUMMARY_H
#define _SIP_MESGSUMMARY_H

#include "sipcommon.h"
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SIP_BY_REFERENCE
extern SipBool sip_mwi_getMesgSummaryFromMsgBody   \
        _ARGS_(( SipMsgBody *msg, MesgSummaryMessage *mwi, SipError *err));
#else
extern SipBool sip_mwi_getMesgSummaryFromMsgBody   \
        _ARGS_(( SipMsgBody *msg, MesgSummaryMessage **mwi, SipError *err));
#endif

extern SipBool sip_mwi_setMesgSummaryInMsgBody  _ARGS_(  \
        ( SipMsgBody *msg, MesgSummaryMessage *mwi, SipError *err));

extern SipBool sip_mwi_getStatusFromMesgSummaryMessage _ARGS_( ( \
	MesgSummaryMessage *pMsgSummary, en_StatusType *pStatusType,\
         SipError *pErr));

extern SipBool sip_mwi_setStatusInMesgSummaryMessage _ARGS_( ( \
        MesgSummaryMessage *pMsgSummary, en_StatusType dStatusType,\
         SipError *pErr));

extern SipBool sip_mwi_getSummaryLineCountFromMesgSummaryMessage _ARGS_( (\
        MesgSummaryMessage *pMsgSummary, SIP_U32bit *count, SipError *err ));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SummaryLine **slSummaryLine,\
         SIP_U32bit index, SipError *err ));
#else
extern SipBool sip_mwi_getSummaryLineAtIndexFromMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine,\
         SIP_U32bit index, SipError *err ));
#endif

extern SipBool sip_mwi_setSummaryLineAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine, \
	SIP_U32bit index, SipError *err ));

extern SipBool sip_mwi_insertSummaryLineAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SummaryLine *slSummaryLine,\
	 SIP_U32bit index, SipError *err ));

extern SipBool sip_mwi_deleteSummaryLineAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SIP_U32bit index, \
	 SipError *err ));

extern SipBool sip_mwi_getNameValuePairCountFromMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SIP_U32bit *count, \
	SipError *err ));

#ifdef SIP_BY_REFERENCE
extern SipBool sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SipNameValuePair **slNameValue,\
         SIP_U32bit index, SipError *err ));
#else
extern SipBool sip_mwi_getNameValuePairAtIndexFromMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
         SIP_U32bit index, SipError *err ));
#endif

extern SipBool sip_mwi_setNameValuePairAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
	 SIP_U32bit index, SipError *err ));

extern SipBool sip_mwi_insertNameValuePairAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SipNameValuePair *slNameValue,\
	 SIP_U32bit index, SipError *err ));

extern SipBool sip_mwi_deleteNameValuePairAtIndexInMesgSummaryMessage _ARGS_( (\
         MesgSummaryMessage *pMsgSummary, SIP_U32bit index, \
	 SipError *err ));

extern SipBool sip_mwi_getMediaFromSummaryLine _ARGS_( (\
         SummaryLine *pSummaryLine, SIP_S8bit **pMedia, SipError *err));

extern SipBool sip_mwi_setMediaInSummaryLine _ARGS_( (\
         SummaryLine *pSummaryLine, SIP_S8bit *pMedia, SipError *err));

/*****Get/set functions for Message-Account header*****/
#ifdef SIP_BY_REFERENCE
extern SipBool sip_mwi_getMesgAccountUrlFromMesgSummaryMessage \
    _ARGS_((MesgSummaryMessage *pMsgSummary, SipAddrSpec **ppAddrSpec,\
            SipError *pErr) );
#else
extern SipBool sip_mwi_getMesgAccountUrlFromMesgSummaryMessage \
    _ARGS_((MesgSummaryMessage *pMsgSummary, SipAddrSpec *pAddrSpec,\
            SipError *pErr) );
#endif

extern SipBool sip_mwi_setMesgAccountUrlInMesgSummaryMessage \
    _ARGS_((MesgSummaryMessage *pMsgSummary, SipAddrSpec *pAddrSpec,\
            SipError *pErr) );

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif

#endif

