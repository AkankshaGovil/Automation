 /*********************************************************************
 **
 ** FILENAME:
 ** sipdecode.c
 **
 ** DESCRIPTION:
 **             This file contains the logic for parsing SIP messages.
 **     It invokes a number of bison generated parsers. It also contains
 **     the logic for indicate invocation.
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 12/01/2000  Binu K S / Arjun RC    						Initial creation
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <ctype.h>
#include "siplist.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "siptrace.h"
#include "sipstatistics.h"
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SIP_TXN_LAYER
#include "siptimer.h"
#endif

#ifdef SIP_IMPP
#include "imppstruct.h"
#include "imppfree.h"
#include "imppinit.h"
#include "imurldecodeintrnl.h"
#endif

#ifdef SIP_CCP
#include "ccpstruct.h"
#include "ccpfree.h"
#include "ccpinit.h"
#endif

#include "rprinit.h"
#include "rprfree.h"
#include "sipbcptinit.h"
#include "sipbcptfree.h"
#include "sipparserclone.h"
#include "sipparserinc.h"

#ifdef SIP_DCS
#include "dcsfree.h"
#include "dcsinit.h"
#include "dcsdecode.h"
#endif

/* All lexers that mantain state have a reset state
        function so that if an error occurs, the stack forcefully
        resets state before entering next parse */

extern void sip_lex_Mime_reset_state(void);
extern void sip_lex_Pgp_reset_state(void);
extern void sip_lex_Via_reset_state(void);

#ifdef SIP_CCP
extern void sip_lex_AcceptContact_reset_state(void);
extern void sip_lex_RejectContact_reset_state(void);
#endif
extern void sip_lex_Contact_reset_state(void);
extern void sip_lex_Fromto_reset_state(void);
extern void sip_lex_To_reset_state(void);
extern void sip_lex_Reqline_reset_state(void);
extern void sip_lex_Sdp_reset_state(void);
#ifdef SIP_DCS
extern void sip_lex_Dcs_reset_state(void); /*DCS*/
#endif

#ifdef SIP_3GPP
extern void sip_lex_3gpp_reset_state(void); /*for 3GPP headers*/
#endif

#ifdef SIP_THREAD_SAFE
synch_id_t      glbLexTokensltokenMutex;
synch_id_t      glbLexTokensMutex;
synch_id_t      glbLexContactMutex;
synch_id_t      glbLexDatetimeMutex;
synch_id_t      glbLexFromtoMutex;
synch_id_t      glbLexToMutex;
synch_id_t      glbLexViaMutex;
synch_id_t      glbLexPgpMutex;
synch_id_t      glbLexUtf8Mutex;
synch_id_t      glbLexTokencommentMutex;
synch_id_t      glbLexAttribMutex;
synch_id_t      glbLexHeaderMutex;
synch_id_t      glbLexMediaMutex;
synch_id_t      glbLexMimeMutex;
synch_id_t      glbLexReqlineMutex;
synch_id_t      glbLexSdpMutex;
#ifdef SIP_MWI
synch_id_t	glbLexMesgSummaryMutex;
#endif
synch_id_t      glbLexStatusMutex;
synch_id_t      glbLexKeyMutex;
synch_id_t      glbLexTelMutex;
synch_id_t      glbLexImMutex;
synch_id_t      glbLexPresMutex;

#ifdef SIP_CCP
synch_id_t      glbLexAcceptContactMutex;
synch_id_t      glbLexRejectContactMutex;
#endif
synch_id_t      glbLexRprTokensMutex;
#ifdef SIP_DCS
synch_id_t      glbLexDcsMutex;
#endif
#ifdef SIP_3GPP
synch_id_t      glbLex3gppMutex;
#endif
#ifdef SIP_CONGEST
synch_id_t      glbLexCongestMutex;
#endif
#endif


#ifdef SIP_MIB
#ifdef SIP_THREAD_SAFE
extern synch_id_t	glbLockRequestStatisticsMutex;
#endif
#endif
#ifdef SIP_THREAD_SAFE
extern synch_id_t	glbLockStatisticsMutex;
#endif


/* Header type stucture used for global selective parse setting */
SipHdrTypeList  glbSipParserHdrTypeList;

void sip_enableAllHeaders
#ifdef ANSI_PROTO
	(SipHdrTypeList *pList)
#else
	(pList)
	SipHdrTypeList *pList;
#endif
{
	SIP_U16bit i=0;
	while(i<((SIP_U16bit)HEADERTYPENUM))
	{
		pList->enable[i++] = SipSuccess;
	}
}

void sip_disableAllHeaders
#ifdef ANSI_PROTO
(SipHdrTypeList *pList)
#else
(pList)
	SipHdrTypeList *pList;
#endif
{
	SIP_U16bit i=0;
	while(i<(SIP_U16bit)HEADERTYPENUM)
	{
		pList->enable[i++] = SipFail;
	}
}

void sip_enableHeader
#ifdef ANSI_PROTO
(SipHdrTypeList *pList, en_HeaderType type)
#else
(pList, type)
	SipHdrTypeList *pList;
	en_HeaderType type;
#endif
{
	if((0<=((int)type))&&(((int)type)<=HEADERTYPENUM))
		pList->enable[(int)type] = SipSuccess;
}

void sip_disableHeader
#ifdef ANSI_PROTO
(SipHdrTypeList *pList, en_HeaderType type)
#else
(pList, type)
	SipHdrTypeList *pList;
	en_HeaderType type;
#endif
{
	if((0<=((int)type))&&(((int)type)<=HEADERTYPENUM))
		pList->enable[(int)type] = SipFail;
}

void sip_enableHeaders
#ifdef ANSI_PROTO
(SipHdrTypeList *pList, en_HeaderType *type, SIP_U16bit size)
#else
(pList, type, size)
	SipHdrTypeList *pList;
	en_HeaderType *type;
	SIP_U16bit size;
#endif
{
	SIP_U16bit i=0;
	while(i<size)
		sip_enableHeader(pList, type[i++]);
}

void sip_disableHeaders
#ifdef ANSI_PROTO
(SipHdrTypeList *pList, en_HeaderType *type, SIP_U16bit size)
#else
(pList, type, size)
	SipHdrTypeList *pList;
	en_HeaderType *type;
	SIP_U16bit size;
#endif
{
	SIP_U16bit i=0;
	while(i<size)
		sip_disableHeader(pList, type[i++]);
}

void sip_stackSetDecodeHeaderTypes
#ifdef ANSI_PROTO
(SipHdrTypeList *pList)
#else
(pList)
	SipHdrTypeList *pList;
#endif
{
	SIP_U16bit i;
	if(pList == SIP_NULL)
		return;
	for(i=0;i<((SIP_U16bit)HEADERTYPENUM);i++)
		glbSipParserHdrTypeList.enable[i]=pList->enable[i];
}


void (*glbSipParserReleaseBuffer[HEADERTYPENUM])(void) =
{
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Key_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Fromto_release_buffer,
	sip_lex_Fromto_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_To_release_buffer,
	sip_lex_Via_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Pgp_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Utf8_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Pgp_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Fromto_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Key_release_buffer,
	sip_lex_Utf8_release_buffer,
	sip_lex_Tokencomment_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Pgp_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Datetime_release_buffer,
	sip_lex_Tokencomment_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Pgp_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	SIP_NULL,
	sip_lex_Tokensltoken_release_buffer,
#ifdef SIP_CCP
	sip_lex_AcceptContact_release_buffer,
	sip_lex_RejectContact_release_buffer,
	sip_lex_Tokens_release_buffer,
#endif
	sip_lex_RprTokens_release_buffer,
	sip_lex_RprTokens_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Key_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Fromto_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Fromto_release_buffer,
	sip_lex_Contact_release_buffer,
	sip_lex_Contact_release_buffer,
#ifdef SIP_IMPP
	sip_lex_Tokensltoken_release_buffer,
	sip_lex_Tokens_release_buffer,
	sip_lex_Datetime_release_buffer,
#endif
#ifdef SIP_DCS
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
	sip_lex_Dcs_release_buffer,
#endif
#ifdef SIP_SESSIONTIMER
	sip_lex_Datetime_release_buffer,
	sip_lex_Datetime_release_buffer,
#endif
	sip_lex_Datetime_release_buffer,
#ifdef SIP_PRIVACY
	sip_lex_Fromto_release_buffer, /* Added for P Asserted Identity */
	sip_lex_Fromto_release_buffer, /* Added for P Preferred Identity */
	sip_lex_Tokens_release_buffer, /* Privacy Header */
#endif
#ifdef SIP_3GPP		
	sip_lex_Fromto_release_buffer, /* Added for Path */
	sip_lex_Fromto_release_buffer, /* Added for P-Access-Network-Info */
	sip_lex_Fromto_release_buffer, /* Added for P-Charging-Vector */
#endif
	sip_lex_Tokensltoken_release_buffer, /* Added for Reason*/
#ifdef SIP_CONGEST
	sip_lex_congest_release_buffer, /* Resource-priority Header */
	sip_lex_congest_release_buffer, /* Accept-Resource-priority Header */
#endif
#ifdef SIP_CONF    
	sip_lex_Contact_release_buffer, /* Join header */
#endif
#ifdef SIP_3GPP		
	sip_lex_3gpp_release_buffer, /*P-Associated-URI 3GPP header */
	sip_lex_3gpp_release_buffer, /*P-Called-Party-Id 3GPP header */
	sip_lex_3gpp_release_buffer, /*P-Visited-Network-Id 3GPP header */
	sip_lex_3gpp_release_buffer, /*P-Charging-Function-Addresses 3GPP header */
#endif
#ifdef SIP_SECURITY
	sip_lex_Via_release_buffer, /*Security Client */
	sip_lex_Via_release_buffer, /*Security Server */
	sip_lex_Via_release_buffer, /*Security Verify */
#endif
#ifdef SIP_3GPP
	sip_lex_Fromto_release_buffer, /*ServiceRoute*/
#endif

	SIP_NULL
};

int (*glbSipParserScanBuffer[HEADERTYPENUM])(SIP_S8bit *, SIP_U32bit) =
{
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Key_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Fromto_scan_buffer,
	sip_lex_Fromto_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_To_scan_buffer,
	sip_lex_Via_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Pgp_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Utf8_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Pgp_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Fromto_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Key_scan_buffer,
	sip_lex_Utf8_scan_buffer,
	sip_lex_Tokencomment_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Pgp_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Datetime_scan_buffer,
	sip_lex_Tokencomment_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Pgp_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	SIP_NULL,
	sip_lex_Tokensltoken_scan_buffer,
#ifdef SIP_CCP
	sip_lex_AcceptContact_scan_buffer,
	sip_lex_RejectContact_scan_buffer,
	sip_lex_Tokens_scan_buffer,
#endif
	sip_lex_RprTokens_scan_buffer,
	sip_lex_RprTokens_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Key_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Fromto_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Fromto_scan_buffer,
	sip_lex_Contact_scan_buffer,
	sip_lex_Contact_scan_buffer,
#ifdef SIP_IMPP
	sip_lex_Tokensltoken_scan_buffer,
	sip_lex_Tokens_scan_buffer,
	sip_lex_Datetime_scan_buffer,
#endif
#ifdef SIP_DCS
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
	sip_lex_Dcs_scan_buffer,
#endif
#ifdef SIP_SESSIONTIMER
	sip_lex_Datetime_scan_buffer,
	sip_lex_Datetime_scan_buffer,
#endif
	sip_lex_Datetime_scan_buffer,
#ifdef SIP_PRIVACY		
	sip_lex_Fromto_scan_buffer, /*P_Asserted Id */
	sip_lex_Fromto_scan_buffer, /*P-Preferred Id */
	sip_lex_Tokens_scan_buffer, /* Privacy hdr */
#endif		
#ifdef SIP_3GPP		
	sip_lex_Fromto_scan_buffer,
	sip_lex_Fromto_scan_buffer, /* P-Access-Network-Info */
	sip_lex_Fromto_scan_buffer,
#endif		
	sip_lex_Tokensltoken_scan_buffer,	/* Reason Hdr */
#ifdef SIP_CONGEST
	sip_lex_congest_scan_buffer, /* Resource-priority Header */
	sip_lex_congest_scan_buffer, /* Accept-Resource-priority Header */
#endif
#ifdef SIP_CONF    
	sip_lex_Contact_scan_buffer, /* Join Header */
#endif
#ifdef SIP_3GPP		
	sip_lex_3gpp_scan_buffer, /*P-Associated-URI */
	sip_lex_3gpp_scan_buffer, /*P-Called-Party-Id */
	sip_lex_3gpp_scan_buffer, /*P-Visited-Network-Id */
	sip_lex_3gpp_scan_buffer, /*P-Charging-Function-Addresses */
#endif		
#ifdef SIP_SECURITY
	sip_lex_Via_scan_buffer, /*Security-Client*/
	sip_lex_Via_scan_buffer, /*Security-Server*/
	sip_lex_Via_scan_buffer, /*Security-Verify*/
#endif
#ifdef SIP_3GPP
	sip_lex_Fromto_scan_buffer, /*Service-Route*/
#endif
	SIP_NULL
};

int (*glbSipParserLexer[HEADERTYPENUM])(SipTokenBuffer *) =
{
        glbSipParserTokensltokenlex,
        glbSipParserTokenslex,
        glbSipParserTokensltokenlex,
        glbSipParserContactlex,
        glbSipParserTokenslex,
        glbSipParserDatetimelex,
        glbSipParserKeylex,
        glbSipParserDatetimelex,
        glbSipParserDatetimelex,
        glbSipParserDatetimelex,
        glbSipParserFromtolex,
        glbSipParserFromtolex,
        glbSipParserDatetimelex,
        glbSipParserTolex,
        glbSipParserVialex,
        glbSipParserTokenslex,
        glbSipParserTokenslex,
        glbSipParserTokensltokenlex,
        glbSipParserPgplex,
        glbSipParserContactlex,
        glbSipParserContactlex,
        glbSipParserContactlex,
        glbSipParserTokenslex,
        glbSipParserTokenslex,
        glbSipParserUtf8lex,
        glbSipParserTokenslex,
        glbSipParserPgplex,
        glbSipParserTokenslex,
        glbSipParserFromtolex,
        glbSipParserTokenslex,
        glbSipParserKeylex,
        glbSipParserUtf8lex,
        glbSipParserTokencommentlex,
        glbSipParserTokenslex,
        glbSipParserPgplex,
        glbSipParserDatetimelex,
        glbSipParserDatetimelex,
        glbSipParserDatetimelex,
        glbSipParserTokencommentlex,
        glbSipParserTokenslex,
        glbSipParserTokensltokenlex,
        glbSipParserPgplex,
        glbSipParserTokensltokenlex,
        SIP_NULL,
        glbSipParserTokensltokenlex,
#ifdef SIP_CCP
        glbSipParserAcceptContactlex,
        glbSipParserRejectContactlex,
        glbSipParserTokenslex,
#endif
        glbSipParserRprTokenslex,
        glbSipParserRprTokenslex,
        glbSipParserTokenslex,
        glbSipParserTokensltokenlex,
        glbSipParserTokensltokenlex,
        glbSipParserTokensltokenlex,
        glbSipParserKeylex,
        glbSipParserTokensltokenlex,
        glbSipParserTokensltokenlex,
        glbSipParserFromtolex,
        glbSipParserContactlex,
        glbSipParserFromtolex,
        glbSipParserContactlex,
        glbSipParserContactlex,
#ifdef SIP_IMPP
        glbSipParserTokensltokenlex,
        glbSipParserTokenslex,
        glbSipParserDatetimelex,
#endif
#ifdef SIP_DCS
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
        glbSipParserDcslex,
#endif
#ifdef SIP_SESSIONTIMER
        glbSipParserDatetimelex,
        glbSipParserDatetimelex,
#endif
        glbSipParserDatetimelex,
#ifdef SIP_PRIVACY		
        glbSipParserFromtolex, /*P-PAsserted-Id */
        glbSipParserFromtolex, /*P-Preferred-Id */
		glbSipParserTokenslex, /* privacy header */
#endif
#ifdef SIP_3GPP		
        glbSipParserFromtolex,
		glbSipParserFromtolex, /* P-Access-Network-Info */
		glbSipParserFromtolex, 
#endif
		glbSipParserTokensltokenlex,	/* Reason Hdr */
#ifdef SIP_CONGEST
	    glbSipParserCongestlex, /* Resource-priority Header */
    	glbSipParserCongestlex, /* Accept-Resource-priority Header */
#endif
#ifdef SIP_CONF        
        glbSipParserContactlex, /* Join Header */
#endif
#ifdef SIP_3GPP		
        glbSipParser3gpplex, /*P-Associated-URI */
        glbSipParser3gpplex, /*P-Called-Party-Id */
        glbSipParser3gpplex, /*P-Visited-Network-Id */
        glbSipParser3gpplex, /*P-Charging-Function-Addresses */
#endif
#ifdef SIP_SECURITY
	glbSipParserVialex,  /*Security-Client*/
	glbSipParserVialex,  /*Security-Server*/
	glbSipParserVialex,  /*Security-Verify*/	
#endif
#ifdef SIP_3GPP
	glbSipParserFromtolex,  /*ServiceRoute*/
#endif
        SIP_NULL
};

/* This array of function pointers contain reset functions for parsers */

void (*glbSipParserReset[HEADERTYPENUM])(void) =
{
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Contact_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Fromto_reset_state,
        sip_lex_Fromto_reset_state,
        SIP_NULL,
        sip_lex_To_reset_state,
        sip_lex_Via_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Pgp_reset_state,
        sip_lex_Contact_reset_state,
        sip_lex_Contact_reset_state,
        sip_lex_Contact_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Pgp_reset_state,
        SIP_NULL,
        sip_lex_Fromto_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Pgp_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Pgp_reset_state,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
#ifdef SIP_CCP
        sip_lex_AcceptContact_reset_state,
        sip_lex_RejectContact_reset_state,
        SIP_NULL,
#endif
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
        sip_lex_Fromto_reset_state,
        sip_lex_Contact_reset_state,
        sip_lex_Fromto_reset_state,
        sip_lex_Contact_reset_state,
        sip_lex_Contact_reset_state,
#ifdef SIP_IMPP
        SIP_NULL,
        SIP_NULL,
        SIP_NULL,
#endif
#ifdef SIP_DCS
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
        sip_lex_Dcs_reset_state,
#endif
#ifdef SIP_SESSIONTIMER
        SIP_NULL,
        SIP_NULL,
#endif
        SIP_NULL,
#ifdef SIP_PRIVACY
        SIP_NULL,   /* P-Asserted Id */
        SIP_NULL,   /* P-Preferred Id */
		SIP_NULL,	/* privacy header */
#endif
#ifdef SIP_3GPP		
        SIP_NULL,
		SIP_NULL,
		SIP_NULL,
#endif
		SIP_NULL,	/* Reason Hdr */
#ifdef SIP_CONGEST
	    SIP_NULL,  /* Resource-priority Header */
    	SIP_NULL, /* Accept-Resource-priority Header */
#endif
#ifdef SIP_CONF        
        sip_lex_Contact_reset_state, /* Join header */
#endif
#ifdef SIP_3GPP		
        sip_lex_3gpp_reset_state, /* P-Associated-URI header*/
        sip_lex_3gpp_reset_state, /* P-Called-Party-Id header*/
        sip_lex_3gpp_reset_state, /* P-Visited-Network-Id header*/
        sip_lex_3gpp_reset_state, /* P-Charging-Function-Addresses header*/
#endif
#ifdef SIP_SECURITY
	sip_lex_Via_reset_state,/*Security-Client*/
	sip_lex_Via_reset_state,/*Security-Server*/
	sip_lex_Via_reset_state,/*Security-Verify*/
#endif
#ifdef SIP_3GPP
	sip_lex_Fromto_reset_state,/*Security-Client*/
#endif
        SIP_NULL
}
;

int (*glbSipParserParser[HEADERTYPENUM])(void *pParserStruct) =
{
        glbSipParserTokensltokenparse,
        glbSipParserTokensparse,
        glbSipParserTokensltokenparse,
        glbSipParserContactparse,
        glbSipParserTokensparse,
        glbSipParserDatetimeparse,
        glbSipParserKeyparse,
        glbSipParserDatetimeparse,
        glbSipParserDatetimeparse,
        glbSipParserDatetimeparse,
        glbSipParserFromtoparse,
        glbSipParserFromtoparse,
        glbSipParserDatetimeparse,
        glbSipParserToparse,
        glbSipParserViaparse,
        glbSipParserTokensparse,
        glbSipParserTokensparse,
        glbSipParserTokensltokenparse,
        glbSipParserPgpparse,
        glbSipParserContactparse,
        glbSipParserContactparse,
        glbSipParserContactparse,
        glbSipParserTokensparse,
        glbSipParserTokensparse,
        glbSipParserUtf8parse,
        glbSipParserTokensparse,
        glbSipParserPgpparse,
        glbSipParserTokensparse,
        glbSipParserFromtoparse,
        glbSipParserTokensparse,
        glbSipParserKeyparse,
        glbSipParserUtf8parse,
        glbSipParserTokencommentparse,
        glbSipParserTokensparse,
        glbSipParserPgpparse,
        glbSipParserDatetimeparse,
        glbSipParserDatetimeparse,
        glbSipParserDatetimeparse,
        glbSipParserTokencommentparse,
        glbSipParserTokensparse,
        glbSipParserTokensltokenparse,
        glbSipParserPgpparse,
        glbSipParserTokensltokenparse,
        SIP_NULL,
        glbSipParserTokensltokenparse,
#ifdef SIP_CCP
        glbSipParserAcceptContactparse,
        glbSipParserRejectContactparse,
        glbSipParserTokensparse,
#endif
        glbSipParserRprTokensparse,
        glbSipParserRprTokensparse,
        glbSipParserTokensparse,
        glbSipParserTokensltokenparse,
        glbSipParserTokensltokenparse,
        glbSipParserTokensltokenparse,
        glbSipParserKeyparse,
        glbSipParserTokensltokenparse,
        glbSipParserTokensltokenparse,
        glbSipParserFromtoparse,
        glbSipParserContactparse,
        glbSipParserFromtoparse,
        glbSipParserContactparse,
        glbSipParserContactparse,
#ifdef SIP_IMPP
        glbSipParserTokensltokenparse,
        glbSipParserTokensparse,
        glbSipParserDatetimeparse,
#endif
#ifdef SIP_DCS
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
        glbSipParserDcsparse,
#endif
#ifdef SIP_SESSIONTIMER
        glbSipParserDatetimeparse,
        glbSipParserDatetimeparse,
#endif
        glbSipParserDatetimeparse,
#ifdef SIP_PRIVACY		
		glbSipParserFromtoparse,/*P-Asserted-Id*/
		glbSipParserFromtoparse,/*P-Preferred-Id */
		glbSipParserTokensparse, /* privacy */
#endif
#ifdef SIP_3GPP		
		glbSipParserFromtoparse,
		glbSipParserFromtoparse, /* P-Access-Network-Info */
		glbSipParserFromtoparse,
#endif
		glbSipParserTokensltokenparse,	/* Reason Hdr*/
#ifdef SIP_CONGEST
	    glbSipParserCongestparse,  /* Resource-priority Header */
    	glbSipParserCongestparse, /* Accept-Resource-priority Header */
#endif
#ifdef SIP_CONF        
        glbSipParserContactparse, /* Join Header */
#endif
#ifdef SIP_3GPP		
		glbSipParser3gppparse,/*P-Associated-URI */
		glbSipParser3gppparse,/*P-Called-Party-Id */
		glbSipParser3gppparse,/*P-Visited-Network-Id */
		glbSipParser3gppparse,/*P-Charging-Function-Addresses */
#endif
#ifdef SIP_SECURITY
	glbSipParserViaparse, /*Security-Client*/
	glbSipParserViaparse, /*Security-Server*/
	glbSipParserViaparse, /*Security-Verify*/
#endif
#ifdef SIP_3GPP
	glbSipParserFromtoparse, /*ServiceRoute*/
#endif
        SIP_NULL
};

 /****************************************************************************
 ** FUNCTION: sip_tokenBufferDup
 *****************************************************************************
 **
 ** Returns an allocated string from the contents of a token buffer used
 ** by the lexer to pass tokens to the parser.
 **
 ****************************************************************************/

SIP_S8bit * sip_tokenBufferDup ( SipTokenBuffer dTokenBuffer)
{
        SIP_S8bit *pRetVal = SIP_NULL;

        /* This is only called by BISON, so id is not of DECODE */
        pRetVal = (SIP_S8bit *) fast_memget(BISON_MEM_ID, \
				dTokenBuffer.dLength+1,SIP_NULL);
        if(pRetVal == SIP_NULL)
                return SIP_NULL;
        strncpy (pRetVal, dTokenBuffer.pToken, dTokenBuffer.dLength);
        pRetVal[dTokenBuffer.dLength] = '\0';
        return pRetVal;
}

/*****************************************************************************
 ** FUNCTION: sip_releaseStack
 *****************************************************************************
 **
 ** Deletes all lexer and parser global allocations
 **
 ****************************************************************************/

void sip_releaseStack()
{

		sip_releaseStackPortSpecific();
		
#ifdef SIP_THREAD_SAFE
        fast_free_synch(&glbLexTokensltokenMutex);
        fast_free_synch(&glbLexTokensMutex);
        fast_free_synch(&glbLexContactMutex);
        fast_free_synch(&glbLexDatetimeMutex);
        fast_free_synch(&glbLexKeyMutex);
        fast_free_synch(&glbLexFromtoMutex);
        fast_free_synch(&glbLexToMutex);
        fast_free_synch(&glbLexViaMutex);
        fast_free_synch(&glbLexPgpMutex);
        fast_free_synch(&glbLexUtf8Mutex);
        fast_free_synch(&glbLexTokencommentMutex);
        fast_free_synch(&glbLexAttribMutex);
        fast_free_synch(&glbLexHeaderMutex);
        fast_free_synch(&glbLexMediaMutex);
        fast_free_synch(&glbLexMimeMutex);
        fast_free_synch(&glbLexReqlineMutex);
        fast_free_synch(&glbLexSdpMutex);
		fast_free_synch(&glbLexTelMutex);
		fast_free_synch(&glbLexImMutex);
#ifdef SIP_MWI
		fast_free_synch(&glbLexMesgSummaryMutex);
#endif
        fast_free_synch(&glbLexStatusMutex);
#ifdef SIP_CCP
        fast_free_synch(&glbLexAcceptContactMutex);
        fast_free_synch(&glbLexRejectContactMutex);
#endif
        fast_free_synch(&glbLexRprTokensMutex);
#ifdef SIP_DCS
        fast_free_synch(&glbLexDcsMutex);
#endif
#ifdef SIP_3GPP
        fast_free_synch(&glbLex3gppMutex);
#endif
#ifdef SIP_CONGEST
        fast_free_synch(&glbLexCongestMutex);
#endif
#endif

#ifdef SIP_THREAD_SAFE
		fast_free_synch(&glbLockStatisticsMutex);
#endif


#ifdef SIP_CCP
        sip_lex_AcceptContact_free();
        sip_lex_RejectContact_free();
#endif
        sip_lex_Attrib_free();
        sip_lex_Contact_free();
        sip_lex_Datetime_free();
        sip_lex_Fromto_free();
        sip_lex_Key_free();
        sip_lex_Media_free();
        sip_lex_Mime_free();
        sip_lex_Pgp_free();
        sip_lex_Reqline_free();
        sip_lex_RprTokens_free();
        sip_lex_Sdp_free();
        sip_lex_Statusline_free();
        sip_lex_Tokencomment_free();
        sip_lex_Tokens_free();
        sip_lex_Tokensltoken_free();
        sip_lex_Utf8_free();
        sip_lex_Via_free();
#ifdef SIP_3GPP
        sip_lex_3gpp_free();
#endif
#ifdef SIP_CONGEST
        sip_lex_congest_free();
#endif
#ifdef SIP_MIB
		__sip_hashFlush();
#ifdef SIP_THREAD_SAFE
		fast_free_synch(&glbLockRequestStatisticsMutex);
#endif
#endif
}

/*****************************************************************************
 ** FUNCTION: sip_initStack
 *****************************************************************************
 **
 ** Initialises various stack parameters
 **
 ****************************************************************************/

SipBool sip_initStack()
{
#ifdef SIP_STATISTICS
        SipError error;
#endif
		sip_initStackPortSpecific();
		
#ifdef SIP_THREAD_SAFE
        fast_init_synch(&glbLexTokensltokenMutex);
        fast_init_synch(&glbLexTokensMutex);
        fast_init_synch(&glbLexContactMutex);
        fast_init_synch(&glbLexDatetimeMutex);
        fast_init_synch(&glbLexKeyMutex);
        fast_init_synch(&glbLexFromtoMutex);
        fast_init_synch(&glbLexToMutex);
        fast_init_synch(&glbLexViaMutex);
        fast_init_synch(&glbLexPgpMutex);
        fast_init_synch(&glbLexUtf8Mutex);
        fast_init_synch(&glbLexTokencommentMutex);
        fast_init_synch(&glbLexAttribMutex);
        fast_init_synch(&glbLexHeaderMutex);
        fast_init_synch(&glbLexMediaMutex);
        fast_init_synch(&glbLexMimeMutex);
        fast_init_synch(&glbLexReqlineMutex);
        fast_init_synch(&glbLexSdpMutex);
		fast_init_synch(&glbLexTelMutex);
		fast_init_synch(&glbLexImMutex);
#ifdef SIP_MWI
	fast_init_synch(&glbLexMesgSummaryMutex);
#endif
        fast_init_synch(&glbLexStatusMutex);
#ifdef SIP_CCP
        fast_init_synch(&glbLexAcceptContactMutex);
        fast_init_synch(&glbLexRejectContactMutex);
#endif
        fast_init_synch(&glbLexRprTokensMutex);
#ifdef SIP_DCS
        fast_init_synch(&glbLexDcsMutex);
#endif
#ifdef SIP_3GPP
        fast_init_synch(&glbLex3gppMutex);
#endif
#endif
#ifdef SIP_MIB
#ifdef SIP_THREAD_SAFE
		fast_init_synch(&glbLockRequestStatisticsMutex);
#endif
#endif
#ifdef SIP_THREAD_SAFE
		fast_init_synch(&glbLockStatisticsMutex);
#endif

#ifdef SIP_STATISTICS
        sip_initStatistics(SIP_STAT_TYPE_API, SIP_STAT_API_ALL,&error);
        sip_initStatistics(SIP_STAT_TYPE_ERROR, SIP_STAT_ERROR_ALL, &error);
#endif

        /* These values are in milliseconds */

        SIP_T1 = SIP_DEFAULT_T1;
        SIP_T2 = SIP_DEFAULT_T2;
        SIP_MAXRETRANS = SIP_DEFAULT_MAX_RETRANS;
        SIP_MAXINVRETRANS = SIP_DEFAULT_INV_RETRANS;
        /* Set default to parse all known headers */
        sip_enableAllHeaders(&glbSipParserHdrTypeList);
        return SipSuccess;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserHeaderNameStrip
 *****************************************************************************
 ** Function to remove whitespaces from pHeader names in the list created by
 ** headerparse funtion. Whitespaces need to be removed for the logic
 ** dLevel parsing done in parseEachHeader
 ****************************************************************************/

void glbSipParserHeaderNameStrip
#ifdef ANSI_PROTO
(SIP_Pvoid pData,SIP_Pvoid pParserParam)
#else
(pData,pParserParam)
	SIP_Pvoid pParserParam;
	SIP_Pvoid pData;
#endif
{
	SIP_U32bit i,j;
	SIP_S8bit *hdrname;
	SIP_S8bit *newname;
	SIP_U32bit hdrlength;
	SipHeaderSplitParserParam *pParserStruct;

	pParserStruct =(SipHeaderSplitParserParam *)pParserParam;
	if(*(pParserStruct->pError)!=E_NO_ERROR)
		return;
	hdrname = (SIP_S8bit *)((SipUnknownHeader *)pData)->pName;
	newname = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
			sizeof(SIP_S8bit)*(strlen(hdrname)+1),SIP_NULL);
	if(newname==SIP_NULL)
	{
		*(pParserStruct->pError) = E_NO_MEM;
		return;
	}
	hdrlength=strlen(hdrname);
	for(i=0,j=0;i<hdrlength;i++)
		if((hdrname[i]==' ')||(hdrname[i]=='\t')||(hdrname[i]=='\n')||\
				(hdrname[i]=='\r'))
		{
		}
		else
		{
			newname[j]=hdrname[i];
			j++;
		}
	newname[j]='\0';
	fast_memfree(DECODE_MEM_ID,hdrname,SIP_NULL);
	((SipUnknownHeader *)pData)->pName = (SIP_S8bit *)newname;
}

/*****************************************************************************
 ** FUNCTION: sip_getTypeFromName
 *****************************************************************************
 ** This function maps a string name to the type of the header.
 ** INPUT PARAMETERS:
 **  pName - string containing the name to be matched.
 ** OUTPUT PARAMETERS:
 **  pType - pointer to enumeration that will cojntain the type of the header
 **  pError - Error value.
 ****************************************************************************/
void sip_getTypeFromName
#ifdef ANSI_PROTO
(SIP_S8bit *pName, en_HeaderType *pType, SipError *pError)
#else
(pName,pType,pError)
	SIP_S8bit *pName;
	en_HeaderType *pType;
	SipError *pError;
#endif
{
	SipError *dummy_pError;
	dummy_pError = pError;
	switch(pName[0])
	{
		case 'A':
		case 'a':
			if(strcasecmp("Authentication-Info:",pName)==0)
			{
				*pType = SipHdrTypeAuthenticationInfo;
			}
			else if(strcasecmp("Accept:",pName)==0)
			{
				*pType = SipHdrTypeAccept;
			}
#ifdef SIP_CCP
			else if(strcasecmp("Accept-Contact:",pName)==0)
			{
				*pType = SipHdrTypeAcceptContact;
			}
			else if(strcasecmp("a:",pName)==0)
			{
				*pType = SipHdrTypeAcceptContact;
			}
#endif
			else if(strcasecmp("Accept-Encoding:",pName)==0)
			{
				*pType = SipHdrTypeAcceptEncoding;
			}
			else if(strcasecmp("Accept-Language:",pName)==0)
			{
				*pType = SipHdrTypeAcceptLanguage;
			}
			else if(strcasecmp("Allow:",pName)==0)
			{
				*pType = SipHdrTypeAllow;
			}
#ifdef SIP_IMPP
			else if(strcasecmp("Allow-Events:",pName)==0)
			{
				*pType = SipHdrTypeAllowEvents;
			}
#endif
			else if(strcasecmp("Authorization:",pName)==0)
			{
				*pType = SipHdrTypeAuthorization;
			}
			else if(strcasecmp("Alert-Info:",pName)==0)
			{
				*pType = SipHdrTypeAlertInfo;
			}
			else if(strcasecmp("Also:",pName)==0)
			{
				*pType = SipHdrTypeAlso;
			}
#ifdef SIP_DCS
			else if ((sip_dcs_getTypeFromName_astart(pName,pType,pError))\
					== SipSuccess)
				break;
#endif
#ifdef SIP_CONGEST
			else if (strcasecmp("Accept-Resource-Priority:",pName) == 0)
            {
				*pType = SipHdrTypeAcceptRsrcPriority;
            }
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'b':
		case 'B':
			if(strcasecmp("b:",pName)==0)
			{
				*pType = SipHdrTypeReferredBy;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;

		case 'c':
		case 'C':
			if(strcasecmp("Call-id:",pName)==0)
			{
				*pType = SipHdrTypeCallId;
			}
			else if(strcasecmp("Contact:",pName)==0)
			{
				*pType = SipHdrTypeContactAny;
			}
			else if(strcasecmp("Content-Encoding:",pName)==0)
			{
				*pType = SipHdrTypeContentEncoding;
			}
			else if(strcasecmp("Content-Length:",pName)==0)
			{
				*pType = SipHdrTypeContentLength;
			}
			else if((strcasecmp("Content-Type:",pName)==0)||\
					(strcasecmp("c:",pName)==0))
			{
				*pType = SipHdrTypeContentType;
			}
			else if(strcasecmp("CSeq:",pName)==0)
			{
				*pType = SipHdrTypeCseq;
			}
			else if(strcasecmp("Call-Info:",pName)==0)
			{
				*pType = SipHdrTypeCallInfo;
			}
			else if(strcasecmp("Content-Language:",pName)==0)
			{
				*pType = SipHdrTypeContentLanguage;
			}
			else if(strcasecmp("Content-Disposition:",pName)==0)
			{
				*pType = SipHdrTypeContentDisposition;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'd':
		case 'D':
			if(strcasecmp("Date:",pName)==0)
			{
				*pType = SipHdrTypeDate;
			}
#ifdef SIP_CCP
			else if(strcasecmp("d:",pName)==0)
			{
				*pType = SipHdrTypeRequestDisposition;
			}
#endif
#ifdef SIP_DCS
			else if ((sip_dcs_getTypeFromName_dstart(pName,\
							pType,pError)) == SipSuccess)
				break;
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'E':

		case 'e':
			if(strcasecmp("Encryption:",pName)==0)
			{
				*pType = SipHdrTypeEncryption;
			}
			else if(strcasecmp("Expires:",pName)==0)
			{
				*pType = SipHdrTypeExpiresAny;
			}
			else if(strcasecmp("e:",pName)==0)
			{
				*pType = SipHdrTypeContentEncoding;
			}
			else if(strcasecmp("Error-Info:",pName)==0)
			{
				*pType = SipHdrTypeErrorInfo;
			}
#ifdef SIP_IMPP
			else if (strcasecmp("Event:",pName)==0) 
			{
				*pType = SipHdrTypeEvent;
			}
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'F':
		case 'f':
			if((strcasecmp("From:",pName)==0)||\
					(strcasecmp("f:",pName)==0))
			{
				*pType = SipHdrTypeFrom;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'H':
		case 'h':
			if(strcasecmp("Hide:",pName)==0)
			{
				*pType = SipHdrTypeHide;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'I':
		case 'i':
			if(strcasecmp("i:",pName)==0)
			{
				*pType = SipHdrTypeCallId;
			}
			else if(strcasecmp("In-Reply-To:",pName)==0)
			{
				*pType = SipHdrTypeInReplyTo;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'j':
		case 'J':
            if (strcasecmp("", pName) == 0)
				*pType=SipHdrTypeUnknown;
#ifdef SIP_CONF            
            else if (strcasecmp("Join:", pName) == 0)
            {
				*pType = SipHdrTypeJoin;
            }
#endif
#ifdef SIP_CCP
            else if(strcasecmp("j:",pName)==0)
			{
				*pType = SipHdrTypeRejectContact;
			}
#endif
			else
				*pType=SipHdrTypeUnknown;
			break;
		case 'K':
		case 'k':
			if(strcasecmp("k:",pName)==0)
			{
				*pType = SipHdrTypeSupported;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'l':
		case 'L':
			if(strcasecmp("l:",pName)==0)
			{
				*pType = SipHdrTypeContentLength;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'M':
		case 'm':
			if(strcasecmp("Max-Forwards:",pName)==0)
			{
				*pType = SipHdrTypeMaxforwards;
			}
			else if(strcasecmp("Mime-Version:",pName)==0)
			{
				*pType = SipHdrTypeMimeVersion;
			}
			else if(strcasecmp("m:",pName)==0)
			{
				*pType = SipHdrTypeContactAny;
			}
			else if(strcasecmp("Min-Expires:",pName) == 0 )
			{
				*pType = SipHdrTypeMinExpires;
			}
#ifdef SIP_SESSIONTIMER
			else if(strcasecmp("Min-SE:",pName) == 0 )
			{
				*pType = SipHdrTypeMinSE;
			}
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'O':
		case 'o':
			if(strcasecmp("Organization:",pName)==0)
			{
				*pType = SipHdrTypeOrganization;
			}
#ifdef SIP_IMPP
			else if (strcasecmp("o:",pName) == 0 )
			{
				*pType = SipHdrTypeEvent;
			}
#endif			
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'P':
		case 'p':
			if(strcasecmp("Priority:",pName)==0)
			{
				*pType = SipHdrTypePriority;
			}
			else if(strcasecmp("Proxy-Authenticate:",pName)==0)
			{
				*pType = SipHdrTypeProxyAuthenticate;
			}
			else if(strcasecmp("Proxy-Authorization:",pName)==0)
			{
				*pType = SipHdrTypeProxyauthorization;
			}
			else if(strcasecmp("Proxy-require:",pName)==0)
			{
				*pType = SipHdrTypeProxyRequire;
			}
#ifdef SIP_PRIVACY
			else if(strcasecmp("P-Asserted-Identity:",pName)
					==0) 
			{
				*pType =SipHdrTypePAssertId;
			}
			else if(strcasecmp("P-Preferred-Identity:",pName)
					==0) 
			{
				*pType =SipHdrTypePPreferredId;
			}
			else if(strcasecmp("Privacy:",pName)==0)
			{
				*pType = SipHdrTypePrivacy;
			}
#endif /* # ifdef SIP_PRIVACY */
#ifdef SIP_3GPP
			else if(strcasecmp("Path:",pName) == 0 )
			{
				*pType =SipHdrTypePath;
			}
			else if(strcasecmp("P-Access-Network-Info:",pName)==0)
			{
				*pType =SipHdrTypePanInfo;
			}	
	        else if(strcasecmp("P-Charging-Vector:",pName)==0)
			{
				*pType =SipHdrTypePcVector;
			}	

#endif
#ifdef SIP_DCS
			else if ((sip_dcs_getTypeFromName_pstart(pName,pType,\
							pError))== SipSuccess)
				break;
#endif
#ifdef SIP_3GPP
			else if(strcasecmp("P-Associated-URI:",pName) == 0 )
			{
				*pType = SipHdrTypePAssociatedUri;
			}
			else if(strcasecmp("P-Called-Party-Id:",pName) == 0 )
			{
				*pType = SipHdrTypePCalledPartyId;
			}
			else if(strcasecmp("P-Visited-Network-Id:",pName) == 0 )
			{
				*pType = SipHdrTypePVisitedNetworkId;
			}
			else if(strcasecmp("P-Charging-Function-Addresses:",pName) == 0 )
			{
				*pType = SipHdrTypePcfAddr;
			}
#endif /*ifdef SIP_3GPP */
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'R':
		case 'r':
			if(strcasecmp("RAck:",pName)==0)
			{
				*pType = SipHdrTypeRAck;
			}
			else if(strcasecmp("Record-Route:",pName)==0)
			{
				*pType = SipHdrTypeRecordRoute;
			}
			else if(strcasecmp("Reason:",pName)==0)
			{
				*pType = SipHdrTypeReason;
			}
#ifdef SIP_CCP
			else if(strcasecmp("Reject-Contact:",pName)==0)
			{
				*pType = SipHdrTypeRejectContact;
			}
			else if(strcasecmp("Request-Disposition:",pName)==0)
			{
				*pType = SipHdrTypeRequestDisposition;
			}
#endif
			else if(strcasecmp("Require:",pName)==0)
			{
				*pType = SipHdrTypeRequire;
			}
			else if(strcasecmp("Response-Key:",pName)==0)
			{
				*pType = SipHdrTypeResponseKey;
			}
			else if(strcasecmp("Retry-After:",pName)==0)
			{
				*pType = SipHdrTypeRetryAfterAny;
			}
			else if(strcasecmp("Route:",pName)==0)
			{
				*pType = SipHdrTypeRoute;
			}
			else if(strcasecmp("RSeq:",pName)==0)
			{
				*pType = SipHdrTypeRSeq;
			}
			else if((strcasecmp("Refer-To:",pName)==0)||\
					(strcasecmp("r:",pName)==0))
			{
				*pType = SipHdrTypeReferTo;
			}
			else if(strcasecmp("Referred-By:",pName)==0)
			{
				*pType = SipHdrTypeReferredBy;
			}
			else if(strcasecmp("Replaces:",pName)==0)
			{
				*pType = SipHdrTypeReplaces;
			}
			else if(strcasecmp("Reply-To:",pName)==0)
			{
				*pType = SipHdrTypeReplyTo;
			}

#ifdef SIP_DCS
			else if ((sip_dcs_getTypeFromName_rstart\
						(pName,pType,pError)) == SipSuccess)
				break;
#endif
#ifdef SIP_CONGEST
			else if (strcasecmp("Resource-Priority:",pName) == 0)
            {
				*pType = SipHdrTypeRsrcPriority;
            }
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'S':
		case 's':
			if(strcasecmp("Server:",pName)==0)
			{
				*pType = SipHdrTypeServer;
			}
			else if (strcasecmp("Service-Route:",pName) == 0)
			{
#ifdef SIP_3GPP
				*pType = SipHdrTypeServiceRoute ;	
#endif
			}
			else if((strcasecmp("Subject:",pName)==0)||\
					(strcasecmp("s:",pName)==0))
			{
				*pType = SipHdrTypeSubject;
			}
			else if(strcasecmp("Supported:",pName)==0)
			{
				*pType = SipHdrTypeSupported;
			}
#ifdef SIP_IMPP
			else if(strcasecmp("Subscription-State:",pName)==0)
			{
				*pType = SipHdrTypeSubscriptionState;
			}
#endif
#ifdef SIP_SESSIONTIMER

			else if(strcasecmp("Session-Expires:",pName)==0)
			{
				*pType =SipHdrTypeSessionExpires;
			}
#endif
#ifdef SIP_DCS
			else if ((sip_dcs_getTypeFromName_sstart(pName,pType,\
							pError)) == SipSuccess)
				break;
#endif
#ifdef SIP_SECURITY
			else if(strcasecmp("Security-Client:",pName)==0)
			{
				*pType =SipHdrTypeSecurityClient;
			}
			else if(strcasecmp("Security-Server:",pName)==0)
                        {
                                *pType =SipHdrTypeSecurityServer;
                        }
			else if(strcasecmp("Security-Verify:",pName)==0)
                        {
                                *pType =SipHdrTypeSecurityVerify;
                        }
#endif
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'T':
		case 't':
			if(strcasecmp("Timestamp:",pName)==0)
			{
				*pType = SipHdrTypeTimestamp;
			}
			else if((strcasecmp("To:",pName)==0)||\
					(strcasecmp("t:",pName)==0))
			{
				*pType = SipHdrTypeTo;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'U':
		case 'u':
			if(strcasecmp("Unsupported:",pName)==0)
			{
				*pType = SipHdrTypeUnsupported;
			}
			else if(strcasecmp("User-Agent:",pName)==0)
			{
				*pType = SipHdrTypeUserAgent;
			}
#ifdef SIP_IMPP
			else if (strcasecmp("U:",pName) == 0 )
			{
				*pType = SipHdrTypeAllowEvents;
			}
#endif			
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'V':
		case 'v':
			if((strcasecmp("Via:",pName)==0)||\
					(strcasecmp("v:",pName)==0))
			{
				*pType = SipHdrTypeVia;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
		case 'W':
		case 'w':
			if(strcasecmp("Warning:",pName)==0)
			{
				*pType = SipHdrTypeWarning;
			}
			else if(strcasecmp("WWW-Authenticate:",pName)==0)
			{
				*pType = SipHdrTypeWwwAuthenticate;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;

#ifdef SIP_SESSIONTIMER
		case 'X':
		case 'x':
			if(strcasecmp("x:",pName)==0)
			{
				*pType =SipHdrTypeSessionExpires;
			}
			else
				*pType = SipHdrTypeUnknown;
			break;
#endif

		default:
			*pType = SipHdrTypeUnknown;
	}
}

/*****************************************************************************
 ** FUNCTION: glbSipParserParseEachHeader
 *****************************************************************************
 ** This function parses each line in the SIP message.
 ** It looks up the pName of the pHeader and calls the appropriate parser
 ** function.
 ****************************************************************************/
void glbSipParserParseEachHeader
#ifdef ANSI_PROTO
(SipUnknownHeader *pHeader, SIP_S8bit *pHeaderBuffer, \
 SIP_U32bit scanLength, SipOptions *pOpt, SIP_Pvoid pParserParam)
#else
(pHeader, pHeaderBuffer, scanLength, pOpt, pParserParam)
	SipUnknownHeader *pHeader;
	SIP_S8bit *pHeaderBuffer;
	SIP_U32bit	scanLength;
	SipOptions *pOpt;
	SIP_Pvoid pParserParam;
#endif
{
	SIP_U32bit dBufferLength=0;
	SIP_S8bit *pTemp=SIP_NULL;
	SipUnknownHeader *unknownhdr=SIP_NULL;
	SipParseEachHeaderParam *pParams=SIP_NULL;

	SIP_S8bit unknownFlag=0, badFlag=0;
	SipError dError;
	en_HeaderType headerType;
	SIP_S8bit result;
	SIP_S8bit *pSipParserInput=SIP_NULL;

	SIP_U32bit dTempNameLen = 0;

	/*      This parsing routine will be called if any previous
			parsing generated an error. So return without parsing
			if there has been an error.
	 */
	pParams=(SipParseEachHeaderParam*)pParserParam;
	if(*(pParams->pParserStruct->pError) != E_NO_ERROR)
		return;

	dTempNameLen = strlen(pHeader->pName);
	dBufferLength = strlen(pHeader->pName);
	if (pHeader->pBody !=SIP_NULL) dBufferLength+=strlen(pHeader->pBody);
	dBufferLength+=2;
	if((pOpt->dOption & SIP_OPT_DBLNULLBUFFER) != SIP_OPT_DBLNULLBUFFER)
	{
		pTemp = (SIP_S8bit *) fast_memget\
			(DECODE_MEM_ID, dBufferLength, SIP_NULL);
		if(pTemp==SIP_NULL)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}
		strcpy (pTemp, pHeader->pName);
		if(pHeader->pBody!=SIP_NULL) strcat(pTemp,pHeader->pBody);
		pTemp[dBufferLength-1] = '\0';
	}
	else
	{
		pTemp=pHeaderBuffer;
		dBufferLength = scanLength+2;
	}


	sip_getTypeFromName(pHeader->pName,&headerType, &dError);
	if((headerType!=SipHdrTypeUnknown) &&\
			(pParams->pList->enable[headerType]==SipSuccess))
	{
#ifdef SIP_SELECTIVE_PARSE

		if(pParams->noSelectiveCallBacks == SipSuccess)
			result = 1;
		else
			result = sip_willParseHeader(headerType, pHeader);
#else
		result = 1;
#endif
		if (result == 1) /* user wants to parse it */
		{
			SipError dErr;
			/* Invoke scan_buffer to set up lexical scanner */
			if(glbSipParserScanBuffer[headerType](pTemp, dBufferLength)!=0)
			{
				*(pParams->pParserStruct->pError) = E_NO_MEM;
				fast_memfree(DECODE_MEM_ID, pTemp, SIP_NULL);
				return;
			}
			/*  Call a reset if available */
			if (glbSipParserReset[headerType] != SIP_NULL)
				glbSipParserReset[headerType]();
			/* Invoke the parser */
			pSipParserInput = (SIP_S8bit *) pTemp;
			glbSipParserParser[headerType]((void *)pParams->pParserStruct);
			sip_listDeleteAll((pParams->pParserStruct->pGCList), &dErr);
			/* Release lexer buffer */
			glbSipParserReleaseBuffer[headerType]();
		}
		else if (result == 2) unknownFlag = 1;
	}
	else
		unknownFlag = 1;

	if (pParams->ignoreErrors != 1)
	{
		if(pParams->noSelectiveCallBacks != SipSuccess)
		{
#ifdef SIP_SELECTIVE_PARSE
			/* Now allow him to carry on  *after* an error header */
			if (*(pParams->pParserStruct->pError) != E_NO_ERROR)
			{
				if (sip_acceptIncorrectHeader(headerType, pHeader)\
						== SipSuccess)
						{
							unknownFlag = 1;
							badFlag = 1;
							*(pParams->pParserStruct->pError) = E_NO_ERROR;
							(pParams->pParserStruct->pSipMessage)->\
								dIncorrectHdrsCount++;
						}
			}
#endif
		}
	}
	else
	{
		if (*(pParams->pParserStruct->pError) != E_NO_ERROR)
		{
			if (*(pParams->pParserStruct->pError) == E_NO_MEM)
			{
				return;
			}
			badFlag = 1;
			*(pParams->pParserStruct->pError) = E_NO_ERROR;
			(pParams->pParserStruct->pSipMessage)->dIncorrectHdrsCount++;
		}
	}

	if(unknownFlag)
	{
		/* Unknown pHeader */
		SipHeaderOrderInfo *order;
		SipBool	dResult;
		unknownhdr = (SipUnknownHeader *) fast_memget(DECODE_MEM_ID,\
				sizeof(SipUnknownHeader),SIP_NULL);
		if(unknownhdr==SIP_NULL)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}
		
		if(dTempNameLen > 0)
		{
			pHeader->pName[strlen(pHeader->pName)-1] = '\0';
		}
		unknownhdr->pName = (SIP_S8bit *) STRDUPDECODE(pHeader->pName);
		if(unknownhdr->pName == SIP_NULL)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}

		if(pHeader->pBody!=SIP_NULL)
		{
			unknownhdr->pBody = (SIP_S8bit *)STRDUPDECODE(pHeader->pBody);
			if(unknownhdr->pBody == SIP_NULL)
			{
				*(pParams->pParserStruct->pError) = E_NO_MEM;
				return;
			}
		}
		else
			unknownhdr->pBody = SIP_NULL;

		HSS_INITREF(unknownhdr->dRefCount);
		if(sip_listAppend(&((pParams->pParserStruct->pSipMessage)->\
						pGeneralHdr->slUnknownHdr),unknownhdr,\
					pParams->pParserStruct->pError)==SipFail)
		{
			return;
		}

		dResult=sip_initSipHeaderOrderInfo(&order,\
				pParams->pParserStruct->pError);
		if (dResult!=SipSuccess)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}

		order->dType = SipHdrTypeUnknown;
		order->dTextType = SipFormFull;
		order->dNum = 1;
		if(sip_listAppend(&((pParams->pParserStruct->pSipMessage)-> \
				slOrderInfo),(SIP_Pvoid)order,pParams->pParserStruct->pError)\
				!=SipSuccess)
		{
			return;
		}
	}
	else if(badFlag)
	{
		/* Unknown pHeader */
		SipBadHeader *badhdr = (SipBadHeader *) fast_memget\
			(DECODE_MEM_ID,sizeof(SipBadHeader),SIP_NULL);
		if(badhdr==SIP_NULL)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}
		if(dTempNameLen > 0)
		{
			pHeader->pName[strlen(pHeader->pName)-1] = '\0';
		}
		badhdr->dType = headerType;
		badhdr->pName = (SIP_S8bit *) STRDUPDECODE(pHeader->pName);
		if(badhdr->pName == SIP_NULL)
		{
			*(pParams->pParserStruct->pError) = E_NO_MEM;
			return;
		}

		if(pHeader->pBody!=SIP_NULL)
		{
			badhdr->pBody = (SIP_S8bit *)STRDUPDECODE(pHeader->pBody);
			if(badhdr->pBody == SIP_NULL)
			{
				*(pParams->pParserStruct->pError) = E_NO_MEM;
				return;
			}
		}
		else
			badhdr->pBody = SIP_NULL;

		HSS_INITREF(badhdr->dRefCount);
		if(sip_listAppend(&((pParams->pParserStruct->pSipMessage)->\
						pGeneralHdr->slBadHdr),badhdr,pParams->pParserStruct\
						->pError) ==SipFail)
		{
			return;
		}
	}
	if((pOpt->dOption & SIP_OPT_DBLNULLBUFFER) != SIP_OPT_DBLNULLBUFFER)
		fast_memfree(DECODE_MEM_ID, pTemp, SIP_NULL);
}


/*****************************************************************************
 ** FUNCTION: glbSipParserRemoveFromOrderTable
 *****************************************************************************
 ** This function removes all entries for the given Header Type from the
 ** order table in the SipMessage. This is used when all headers of the Type
 ** are removed from the message structure.
 ****************************************************************************/
SipBool glbSipParserRemoveFromOrderTable
#ifdef ANSI_PROTO
(SipMessage *s, en_HeaderType dType, SipError *err)
#else
(s,dType,err)
	SipMessage *s;
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIP_U32bit count,i;
	SipHeaderOrderInfo *oinfo;

	if(sip_listSizeOf(&s->slOrderInfo, &count, err) != SipSuccess)
		return SipFail;
	i=0;
	while(i<count)
	{
		if(sip_listGetAt(&s->slOrderInfo,i,(SIP_Pvoid *) &oinfo, err)!=\
				SipSuccess)
			return SipFail;
		if(oinfo->dType == dType)
		{
			if(sip_listDeleteAt(&s->slOrderInfo,i,err)!=SipSuccess)
				return SipFail;
			count--;
		}
		else
			i++;
	}
	return SipSuccess;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserRemoveExistingHeaders
 *****************************************************************************
 ** This function is invoked after the Body of an encrypted message has
 ** been decrypted. It takes each pHeader in the decrypted part and removes
 ** those headers from the existing Message structure. This is done to
 ** fulfill the requirement of replacing existing headers with the headers
 ** that are present in the encrypted part of the message.
 ****************************************************************************/
void glbSipParserRemoveExistingHeaders
#ifdef ANSI_PROTO
(SIP_Pvoid pData,SIP_Pvoid pParserParam)
#else
(pData,pParserParam)
	SIP_Pvoid pData;
	SIP_Pvoid pParserParam;
#endif
{
	SIP_U32bit i;
	SipUnknownHeader *yyhdr;
	SIP_S8bit *temp;
	SipHeaderParserParam *pParserStruct;


	/*      This parsing routine will be called if any previous
			parsing generated an error. So return without parsing
			if there has been an error.
	 */
	pParserStruct =(SipHeaderParserParam *)pParserParam;
	if(*(pParserStruct->pError)!= E_NO_ERROR)
		return;

	yyhdr = (SipUnknownHeader *)pData;
	i= strlen(yyhdr->pName)+1;
	if (yyhdr->pBody !=SIP_NULL) i+=strlen(yyhdr->pBody);
	temp = (SIP_S8bit *) fast_memget(DECODE_MEM_ID, i,SIP_NULL);
	if(temp==SIP_NULL)
	{
		*(pParserStruct->pError)= E_NO_MEM;
		return;
	}
	strcpy (temp, yyhdr->pName);
	if(yyhdr->pBody!=SIP_NULL) strcat(temp,yyhdr->pBody);

	if(strcasecmp("Accept:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slAcceptHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAccept,pParserStruct->pError);
	}
	else if(strcasecmp("Accept-Language:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slAcceptLang,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAcceptLanguage,pParserStruct->pError);
	}
	else if((strcasecmp("Content-Type:",yyhdr->pName)==0)||\
			(strcasecmp("c:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pContentTypeHdr!=SIP_NULL)
		{
			sip_freeSipContentTypeHeader((pParserStruct->\
						pSipMessage)-> pGeneralHdr->pContentTypeHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pContentTypeHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContentType,pParserStruct->pError);
	}
	else if((strcasecmp("Content-Language:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slContentLanguageHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContentLanguage,pParserStruct->pError);
	}
	else if((strcasecmp("Content-Disposition:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slContentDispositionHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContentDisposition,pParserStruct->pError);
	}
	else if((strcasecmp("Reason:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slReasonHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeReason,pParserStruct->pError);
	}
	else if(strcasecmp("Accept-Encoding:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slAcceptEncoding,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAcceptEncoding,pParserStruct->pError);
	}
	else if((strcasecmp("Call-id:",yyhdr->pName)==0)||\
			(strcasecmp("i:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pCallidHdr!=SIP_NULL)
		{
			sip_freeSipCallIdHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pCallidHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr\
				->pCallidHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeCallId,pParserStruct->pError);
	}
	else if(strcasecmp("Reply-To:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pReplyToHdr!=SIP_NULL)
		{
			sip_freeSipReplyToHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pReplyToHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pReplyToHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeReplyTo,pParserStruct->pError);
	}
	else if(strcasecmp("CSeq:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pCseqHdr\
				!=SIP_NULL)
		{
			sip_freeSipCseqHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pCseqHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pCseqHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeCseq,pParserStruct->pError);
	}
	else if((strcasecmp("Content-Encoding:",yyhdr->pName)==0)||\
			(strcasecmp("e:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slContentEncodingHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContentEncoding,pParserStruct->pError);
	}
	else if((strcasecmp("Content-Length:",yyhdr->pName)==0)||\
			(strcasecmp("l:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pContentLengthHdr!=SIP_NULL)
		{
			sip_freeSipContentLengthHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pContentLengthHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pContentLengthHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContentLength,pParserStruct->pError);
	}
	else if(strcasecmp("Max-Forwards:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
					pMaxForwardsHdr!=SIP_NULL)
			{
				sip_freeSipMaxForwardsHeader((pParserStruct->pSipMessage)->\
						u.pRequest->pRequestHdr->pMaxForwardsHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
					pMaxForwardsHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeMaxforwards,pParserStruct->pError);
	}
	else if(strcasecmp("Proxy-require:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slProxyRequireHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeProxyRequire,pParserStruct->pError);
	}
	else if(strcasecmp("Require:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slRequireHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRequire,pParserStruct->pError);
	}
	else if(strcasecmp("Allow:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slAllowHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAllow,pParserStruct->pError);
	}
	else if(strcasecmp("Unsupported:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slUnsupportedHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeUnsupported,pParserStruct->pError);
	}
	else if(strcasecmp("Warning:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slWarningHeader,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeWarning,pParserStruct->pError);
	}

	else if(strcasecmp("Retry-After:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pRetryAfterHdr!=SIP_NULL)
		{
			sip_freeSipRetryAfterHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pRetryAfterHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pRetryAfterHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRetryAfterAny,pParserStruct->pError);
	}
	else if(strcasecmp("Replaces:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReplacesHdr!=SIP_NULL)
		{
			sip_freeSipReplacesHeader((pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->pReplacesHdr);
			(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReplacesHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeReplaces,pParserStruct->pError);
	}
#ifdef SIP_CCP
	else if(strcasecmp("Accept-Contact:",yyhdr->pName)==0 || \
			strcasecmp("a:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slAcceptContactHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAcceptContact,pParserStruct->pError);
	}
	else if(strcasecmp("Reject-Contact:",yyhdr->pName)==0 || \
			strcasecmp("j:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slRejectContactHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRejectContact,pParserStruct->pError);
	}
	else if(strcasecmp("Request-Disposition:",yyhdr->pName)==0 || \
			strcasecmp("d:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slRequestDispositionHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRequestDisposition,pParserStruct->pError);
	}
#endif
	else if (strcasecmp("Mime-Version:",yyhdr->pName)==0 )
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pMimeVersionHdr!=SIP_NULL)
		{
			sip_bcpt_freeSipMimeVersionHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pMimeVersionHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pMimeVersionHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeMimeVersion,pParserStruct->pError);
	}
	else if (strcasecmp("RAck:",yyhdr->pName)==0 )
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pRackHdr !=SIP_NULL)
			{
				sip_rpr_freeSipRAckHeader((pParserStruct->\
						pSipMessage)->u.pRequest-> pRequestHdr->\
						pRackHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pRackHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage), \
				SipHdrTypeRAck, pParserStruct->pError);
	}
	else if (strcasecmp("RSeq:",yyhdr->pName)==0 )
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_rpr_freeSipRSeqHeader((pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->pRSeqHdr);
			(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->pRSeqHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRSeq,pParserStruct->pError);
	}
#ifdef SIP_IMPP
	else if(strcasecmp("Event:",yyhdr->pName)==0 || \
			strcasecmp("o:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pEventHdr!=SIP_NULL)
		{
			sip_impp_freeSipEventHeader((pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->pEventHdr);
			(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pEventHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeEvent,pParserStruct->pError);
	}
	else if(strcasecmp("Allow-Events:",yyhdr->pName)==0 || \
			strcasecmp("U:",yyhdr->pName)==0 )
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slAllowEventsHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAllowEvents,pParserStruct->pError);
	}
	else if(strcasecmp("Subscription-State:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pSubscriptionStateHdr !=SIP_NULL)
			{
				sip_impp_freeSipSubscriptionStateHeader((pParserStruct->\
						pSipMessage)->u.pRequest-> pRequestHdr->\
						pSubscriptionStateHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pSubscriptionStateHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage), \
				SipHdrTypeSubscriptionState, pParserStruct->pError);
	}
#endif
	else if(strcasecmp("Expires:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pExpiresHdr\
				!=SIP_NULL)
		{
			sip_freeSipExpiresHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pExpiresHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pExpiresHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeExpiresAny,pParserStruct->pError);
	}
	else if (strcasecmp("Min-Expires:",yyhdr->pName) == 0)
	{
		if((pParserStruct->pSipMessage)->u.pResponse->pResponseHdr->\
				pMinExpiresHdr!=SIP_NULL)
		{
			sip_freeSipMinExpiresHeader((pParserStruct->pSipMessage)->\
					u.pResponse->pResponseHdr->\
					pMinExpiresHdr);
			(pParserStruct->pSipMessage)->u.pResponse->pResponseHdr->\
				pMinExpiresHdr= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeMinExpires,pParserStruct->pError);
	}
#ifdef SIP_SESSIONTIMER
	else if((strcasecmp("Session-Expires:",yyhdr->pName)==0)|| \
			(strcasecmp("x:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->
				pSessionExpiresHdr!=SIP_NULL)
		{
			sip_freeSipSessionExpiresHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pSessionExpiresHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pSessionExpiresHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeSessionExpires,pParserStruct->pError);
	}
	else if (strcasecmp("Min-SE:",yyhdr->pName) == 0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pMinSEHdr!=SIP_NULL)
		{
			sip_freeSipMinSEHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pMinSEHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pMinSEHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeMinSE,pParserStruct->pError);
	}
#endif



	else if(strcasecmp("Timestamp:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pTimeStampHdr\
				!=SIP_NULL)
		{
			sip_freeSipTimeStampHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pTimeStampHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pTimeStampHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeTimestamp,pParserStruct->pError);
	}
	else if(strcasecmp("Date:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pDateHdr!=\
				SIP_NULL)
		{
			sip_freeSipDateHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pDateHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pDateHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeDate,pParserStruct->pError);
	}

	else if((strcasecmp("Subject:",yyhdr->pName)==0)||\
			(strcasecmp("s:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
					pSubjectHdr !=SIP_NULL)
			{
				sip_freeSipSubjectHeader((pParserStruct->pSipMessage)-> \
						u.pRequest-> pRequestHdr->pSubjectHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pSubjectHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeSubject,pParserStruct->pError);
	}
	else if((strcasecmp("Supported:",yyhdr->pName)==0)||\
			(strcasecmp("k:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slSupportedHdr,pParserStruct->pError);

		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeSupported,pParserStruct->pError);
	}
	else if(strcasecmp("Organization:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->\
				pOrganizationHdr!=SIP_NULL)
		{
			sip_freeSipOrganizationHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pOrganizationHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pOrganizationHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeOrganization,pParserStruct->pError);
	}
	else if((strcasecmp("From:",yyhdr->pName)==0)||\
			(strcasecmp("f:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pFromHeader\
				!=SIP_NULL)
		{
			sip_freeSipFromHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pFromHeader);
			(pParserStruct->pSipMessage)->pGeneralHdr->pFromHeader \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeFrom,pParserStruct->pError);
	}
	else if((strcasecmp("To:",yyhdr->pName)==0)||(strcasecmp("t:",yyhdr->pName)\
				==0))
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pToHdr\
				!=SIP_NULL)
		{
			sip_freeSipToHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pToHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pToHdr = \
				SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
					SipHdrTypeTo, pParserStruct->pError);
	}
	else if(strcasecmp("Route:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slRouteHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRoute,pParserStruct->pError);
	}
	else if(strcasecmp("Record-Route:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slRecordRouteHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeRecordRoute,pParserStruct->pError);
	}
	else if(strcasecmp("Authorization:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->slAuthorizationHdr,\
					pParserStruct->pError);
		}
		else if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->slAuthorizationHdr,\
					pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAuthorization,pParserStruct->pError);
	}
	else if(strcasecmp("Proxy-Authorization:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				u.pRequest->pRequestHdr->slProxyAuthorizationHdr,\
				pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeProxyauthorization,pParserStruct->pError);
	}
	else if(strcasecmp("Proxy-Authenticate:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slProxyAuthenticateHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeProxyAuthenticate,pParserStruct->pError);
	}
	else if(strcasecmp("Authentication-Info:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slAuthenticationInfoHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAuthenticationInfo,pParserStruct->pError);
	}
	else if(strcasecmp("WWW-Authenticate:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slWwwAuthenticateHdr,pParserStruct->pError);
		}
		else if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slWwwAuthenticateHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeWwwAuthenticate,pParserStruct->pError);
	}
	else if((strcasecmp("Contact:",yyhdr->pName)==0)||\
			(strcasecmp("m:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slContactHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeContactAny,pParserStruct->pError);
	}
	else if(strcasecmp("Hide:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->pHideHdr\
					!=SIP_NULL)
			{
				sip_freeSipHideHeader((pParserStruct->pSipMessage)-> \
						u.pRequest-> pRequestHdr->pHideHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pHideHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage), \
				SipHdrTypeHide, pParserStruct->pError);
	}
	else if(strcasecmp("Priority:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pPriorityHdr !=SIP_NULL)
			{
				sip_freeSipPriorityHeader((pParserStruct->pSipMessage)-> \
						u.pRequest-> pRequestHdr->pPriorityHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pPriorityHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePriority,pParserStruct->pError);
	}
	else if((strcasecmp("Via:",yyhdr->pName)==0)||\
			(strcasecmp("v:",yyhdr->pName)==0))
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slViaHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeVia,pParserStruct->pError);
	}
	else if(strcasecmp("Response-Key:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pRespKeyHdr !=SIP_NULL)
			{
				sip_freeSipRespKeyHeader((pParserStruct->pSipMessage)-> \
						u.pRequest-> pRequestHdr->pRespKeyHdr);
				(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr-> \
					pRespKeyHdr = SIP_NULL;
			}
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeResponseKey,pParserStruct->pError);
	}
	else if(strcasecmp("Encryption:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pEncryptionHdr\
				!=SIP_NULL)
		{
			sip_freeSipEncryptionHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pEncryptionHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->\
				pEncryptionHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeEncryption,pParserStruct->pError);
	}
	else if(strcasecmp("Server:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_freeSipServerHeader((pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->pServerHdr);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeServer,pParserStruct->pError);
	}
	else if(strcasecmp("Call-Info:",yyhdr->pName)==0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
				pGeneralHdr->slCallInfoHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeCallInfo,pParserStruct->pError);
	}

	else if(strcasecmp("Alert-Info:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slAlertInfoHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAlertInfo,pParserStruct->pError);
	}

	else if((strcasecmp("Refer-To:",yyhdr->pName)==0)||\
			(strcasecmp("r:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReferToHdr!=SIP_NULL)
		{
			sip_freeSipReferToHeader((pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->pReferToHdr);
			(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReferToHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeReferTo,pParserStruct->pError);
	}

	else if((strcasecmp("Referred-By:",yyhdr->pName)==0)||\
			(strcasecmp("b:",yyhdr->pName)==0))
	{
		if((pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReferredByHdr!=SIP_NULL)
		{
			sip_freeSipReferredByHeader((pParserStruct->pSipMessage)->\
					u.pRequest->pRequestHdr->pReferredByHdr);
			(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
				pReferredByHdr = SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeReferredBy,pParserStruct->pError);
	}

	else if(strcasecmp("In-Reply-To:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slInReplyToHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeInReplyTo,pParserStruct->pError);
	}
	else if(strcasecmp("User-Agent:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pUserAgentHdr\
				!=SIP_NULL)
		{
			sip_freeSipUserAgentHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pUserAgentHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pUserAgentHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeUserAgent,pParserStruct->pError);
	}
	else if(strcasecmp("Error-Info:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slErrorInfoHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeErrorInfo,pParserStruct->pError);
	}
	else if(strcasecmp("Also:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
		{
			sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slAlsoHdr,pParserStruct->pError);
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeAlso,pParserStruct->pError);
	}
#ifdef SIP_PRIVACY
	else if(strcasecmp("P-Asserted-Identity:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slPAssertIdHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypePAssertId,pParserStruct->pError);
	} /* for P-Asserted-Id */
	else if(strcasecmp("P-Preferred-Identity:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slPPreferredIdHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypePPreferredId,pParserStruct->pError);
	} /* for P-Preferred-Id */
	else if(strcasecmp("Privacy:",yyhdr->pName) == 0)
	{
		sip_freeSipPrivacyHeader((pParserStruct->pSipMessage)->\
				pGeneralHdr->pPrivacyHdr);
		(pParserStruct->pSipMessage)->pGeneralHdr->pPrivacyHdr = NULL;
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePrivacy,pParserStruct->pError);
	}
#endif /* # ifdef SIP_PRIVACY */
#ifdef SIP_3GPP
	else if (strcasecmp("Service-Route:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slServiceRouteHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypeServiceRoute,pParserStruct->pError);
	}
	else if(strcasecmp("Path:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->pGeneralHdr->\
				slPathHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypePath,pParserStruct->pError);
	} 
	   else if(strcasecmp("P-Access-Network-Info:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pPanInfoHdr\
				!=SIP_NULL)
		{
			sip_freeSipPanInfoHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pPanInfoHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pPanInfoHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePanInfo,pParserStruct->pError);
	}
    else if(strcasecmp("P-Charging-Vector:",yyhdr->pName)==0)
	{
		if((pParserStruct->pSipMessage)->pGeneralHdr->pPcVectorHdr\
				!=SIP_NULL)
		{
			sip_freeSipPcVectorHeader((pParserStruct->pSipMessage)->\
					pGeneralHdr->pPcVectorHdr);
			(pParserStruct->pSipMessage)->pGeneralHdr->pPcVectorHdr \
				= SIP_NULL;
		}
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePcVector,pParserStruct->pError);
	}
#
#endif

#ifdef SIP_CONGEST
	else if(strcasecmp("Resource-Priority:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slRsrcPriorityHdr,pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypeRsrcPriority,pParserStruct->pError);
    }
	else if(strcasecmp("Accept-Resource-Priority:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slAcceptRsrcPriorityHdr,\
                    pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypeAcceptRsrcPriority,pParserStruct->pError);
    }
#endif

#ifdef SIP_CONF
    /* Added for Join header*/
    else if(strcasecmp("Join:",yyhdr->pName) == 0)
	{
		sip_freeSipJoinHeader((pParserStruct->pSipMessage)->\
				u.pRequest->pRequestHdr->pJoinHdr);
		(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->pJoinHdr = NULL;
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypeJoin,pParserStruct->pError);
    }
#endif

#ifdef SIP_3GPP
    else if(strcasecmp("P-Associated-URI:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
					pResponseHdr->slPAssociatedUriHdr,\
                    pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypePAssociatedUri,pParserStruct->pError);
    }
    else if(strcasecmp("P-Called-Party-Id:",yyhdr->pName) == 0)
	{
		sip_freeSipPCalledPartyIdHeader((pParserStruct->pSipMessage)->\
				u.pRequest->pRequestHdr->pPCalledPartyIdHdr);
		(pParserStruct->pSipMessage)->u.pRequest->pRequestHdr->\
            pPCalledPartyIdHdr = NULL;
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePCalledPartyId,pParserStruct->pError);
    }
    else if(strcasecmp("P-Visited-Network-Id:",yyhdr->pName) == 0)
	{
		sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
					pRequestHdr->slPVisitedNetworkIdHdr,\
                    pParserStruct->pError);
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage)\
				,SipHdrTypePVisitedNetworkId,pParserStruct->pError);
    }
   	else if(strcasecmp("P-Charging-Function-Addresses:",yyhdr->pName) == 0)
	{
		sip_freeSipPcfAddrHeader((pParserStruct->pSipMessage)->\
				pGeneralHdr->pPcfAddrHdr);
		(pParserStruct->pSipMessage)->pGeneralHdr->pPcfAddrHdr = NULL;
		glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
				SipHdrTypePcfAddr,pParserStruct->pError);
	}
#endif
#ifdef SIP_SECURITY
        else if(strcasecmp("Security-Client:",yyhdr->pName)==0)
        {
                if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
                {
                        sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
                        pRequestHdr-> slSecurityClientHdr,pParserStruct->pError);
                }
                glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
                        SipHdrTypeSecurityClient,pParserStruct->pError);
        }
	else if(strcasecmp("Security-Server:",yyhdr->pName)==0)
	{
  		if((pParserStruct->pSipMessage)->dType == SipMessageResponse)
  		{
  	      		sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pResponse->\
			pResponseHdr->slSecurityServerHdr,pParserStruct->pError);
  		}
  		glbSipParserRemoveFromOrderTable ((pParserStruct->pSipMessage),\
 			SipHdrTypeSecurityServer,pParserStruct->pError);
	}
	else if(strcasecmp("Security-Verify:",yyhdr->pName)==0)
        {
                if((pParserStruct->pSipMessage)->dType == SipMessageRequest)
                {
                        sip_listDeleteAll(&(pParserStruct->pSipMessage)->u.pRequest->\
			pRequestHdr-> slSecurityVerifyHdr,pParserStruct->pError);
                }
                glbSipParserRemoveFromOrderTable((pParserStruct->pSipMessage),\
 			SipHdrTypeSecurityVerify,pParserStruct->pError);
        }
#endif

#ifdef SIP_DCS
	else
	{
		sip_dcs_glbSipParserRemoveExistingHeaders (yyhdr,\
				(pParserStruct->pSipMessage),*(pParserStruct->pError));
	}
#endif
    
	sip_listDeleteAll(&(pParserStruct->pSipMessage)->\
			pGeneralHdr->slBadHdr,pParserStruct->pError);

	fast_memfree(DECODE_MEM_ID,temp,SIP_NULL);
}



/*****************************************************************************
 ** FUNCTION: glbSipParserSdpHeaderParse
 *****************************************************************************
 ** This function parses each line in the SDP message.
 ** It looks up the Name of the pHeader and calls the appropriate parser
 ** function.
 ****************************************************************************/
void glbSipParserSdpHeaderParse
#ifdef ANSI_PROTO
(SIP_Pvoid pData, SIP_Pvoid pParserParam)
#else
(pData, pParserParam)
	SIP_Pvoid pData;
	SIP_Pvoid pParserParam;
#endif
{
	SIP_S8bit *sdpline, *sdpoutline;
	SIP_S8bit *pSipParserInput;
	SipParseEachSdpLineParam *pSdpParam;
#ifdef SIP_SELECTIVE_PARSE
	SIP_S8bit *IncorrectSdpLine;
#endif
	SIP_U32bit sdpBufferLength;
	SIP_S8bit result;
	SipError tempError;

	/*      This parsing routine will be called if any previous
			parsing generated an error. So return without parsing
			if there has been an error.
	 */
	pSdpParam=(SipParseEachSdpLineParam*)pParserParam;

	result = 1;
	if(*(pSdpParam->pSdpParam->pError) != E_NO_ERROR)
		return;

	sdpline = (SIP_S8bit *) pData;
	sdpBufferLength = strlen(sdpline)+2;
	sdpoutline = SIP_NULL;
#ifdef SIP_SELECTIVE_PARSE

	result = sip_willParseSdpLine(sdpline,&sdpoutline);
	if(result == 1)
	{ /* Parse the message */
		if(sdpoutline != SIP_NULL)
			sdpline = sdpoutline;
	}
	else
	{ /* Throw the line away */
		return;
	}
#endif

	if (sdpline[0]=='m')
		pSdpParam->parserState=0;
	/* if its m, it gets parsed by state 0 rule */

	if(pSdpParam->parserState==0)
	{
		switch(sdpline[0])
		{
			case 'v':
			case 'o':
			case 's':
			case 'i':
			case 'u':
			case 'e':
			case 'p':
			case 'c':
			case 'b':
			case 't':
			case 'k':
				{
					pSdpParam->repeatState = (sdpline[0]=='t');
					if (sip_lex_Sdp_scan_buffer(sdpline, sdpBufferLength)!= 0)
					{
						*(pSdpParam->pSdpParam->pError) = E_NO_MEM;
						return;
					}

					sip_lex_Sdp_reset_state();
					pSipParserInput = sdpline;
					glbSipParserSdpparse((void *)pSdpParam->pSdpParam);
					sip_lex_Sdp_release_buffer();
					break;
				}

			case 'r': if (pSdpParam->repeatState==1)
						  {
							  if (sip_lex_Sdp_scan_buffer(sdpline, \
								  				sdpBufferLength)!= 0)
							  {
								  *(pSdpParam->pSdpParam->pError) = E_NO_MEM;
								  return;
							  }
							  sip_lex_Sdp_reset_state();
							  pSipParserInput = sdpline;
							  glbSipParserSdpparse((void*)pSdpParam->pSdpParam);
							  sip_lex_Sdp_release_buffer();
							  break;
						  }

			case 'a':
					  {
						  pSdpParam->repeatState = (sdpline[0]=='t');
						  if (sip_lex_Attrib_scan_buffer(sdpline, \
							  					sdpBufferLength)!= 0)
						  {
							  *(pSdpParam->pSdpParam->pError) = E_NO_MEM;
							  return;
						  }
						  pSipParserInput = sdpline;
						  glbSipParserAttribparse((void *)pSdpParam->pSdpParam);
						  sip_lex_Attrib_release_buffer();
						  break;
					  }

			case 'm':
					  {
						  pSdpParam->repeatState = (sdpline[0]=='t');
						  pSdpParam->parserState = 1;
						  pSdpParam->pSdpParam->pMedia = (SdpMedia *)\
							  fast_memget(DECODE_MEM_ID,sizeof(SdpMedia),\
							  				SIP_NULL);
						  if(pSdpParam->pSdpParam->pMedia == SIP_NULL)
						  {
							  *(pSdpParam->pSdpParam->pError) = E_NO_MEM;
							  return;
						  }
						  pSdpParam->pSdpParam->pMedia->pInformation = SIP_NULL;
						  pSdpParam->pSdpParam->pMedia->pKey = SIP_NULL;
						  pSdpParam->pSdpParam->pMedia->pFormat = SIP_NULL;
						  pSdpParam->pSdpParam->pMedia->pPortNum = SIP_NULL;
						  pSdpParam->pSdpParam->pMedia->pProtocol = SIP_NULL;
						  pSdpParam->pSdpParam->pMedia->pMediaValue  = SIP_NULL;
#ifdef SIP_ATM
						  pSdpParam->pSdpParam->pMedia->pVirtualCID  = SIP_NULL;
						  if(sip_listInit(&pSdpParam->pSdpParam->pMedia->\
									  slProtofmt, __sip_freeSipNameValuePair,
									  pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return;
						  }
#endif
						  HSS_INITREF(pSdpParam->pSdpParam->pMedia->dRefCount);
						  if(sip_listInit(&pSdpParam->pSdpParam->pMedia->\
							  		  slConnection,__sip_freeSdpConnection,\
									  pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return;
						  }

						  if(sip_listInit(&pSdpParam->pSdpParam->pMedia->\
							 	 	  slBandwidth,__sip_freeString,\
									  pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return;
						  }

						  if(sip_listInit(&pSdpParam->pSdpParam->pMedia->\
							         slAttr,__sip_freeSdpAttr,\
									 pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return ;
						  }

						  if(sip_listInit(&pSdpParam->pSdpParam->pMedia->\
									  slIncorrectLines, __sip_freeString,\
									  pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return ;
						  }

						  if(sip_listAppend(&(pSdpParam->pSdpParam->\
							  		  pSdpMessage->slMedia), \
									  pSdpParam->pSdpParam->pMedia,\
									  pSdpParam->pSdpParam->pError)==SipFail)
						  {
							  return ;
						  }

						  if (sip_lex_Media_scan_buffer(sdpline, \
							  		 sdpBufferLength) != 0)
						  {
							  *(pSdpParam->pSdpParam->pError) = E_NO_MEM;
							  return;
						  }
						  pSipParserInput = sdpline;
						  glbSipParserMediaparse((void *)pSdpParam->pSdpParam);
						  sip_lex_Media_release_buffer();
						  break;
					  }

			case 'z':
					  {
						  if (pSdpParam->repeatState==1)
						  {
							  if (sip_lex_Sdp_scan_buffer(sdpline, \
								  		sdpBufferLength)!= 0)
							  {
								  *(pSdpParam->pSdpParam->pError) = E_NO_MEM;
								  return;
							  }
							  sip_lex_Sdp_reset_state();
							  pSipParserInput = sdpline;
							  glbSipParserSdpparse((void*)pSdpParam->pSdpParam);
							  sip_lex_Sdp_release_buffer();
							  break;
						  }
					  }
			default:
					  {
						  *(pSdpParam->pSdpParam->pError) = E_PARSER_ERROR;
						  sip_error(SIP_Minor, (SIP_S8bit *)\
								  "Session level or invalid attribute found"\
								  " when expecting SDP media attribute.");
					  }
		}       /* switch */
	}
	else
	{
		switch(sdpline[0])
		{
			case 'i':
			case 'c':
			case 'b':
			case 'k':
			case 'a':
				{
					if (sip_lex_Media_scan_buffer(sdpline, sdpBufferLength)!= 0)
					{
						*(pSdpParam->pSdpParam->pError) = E_NO_MEM;
						return;
					}

					pSipParserInput = sdpline;
					glbSipParserMediaparse((void *)pSdpParam->pSdpParam);
					sip_lex_Media_release_buffer();
					break;
				}
			default:
				{
					sip_error(SIP_Minor,(SIP_S8bit *)\
							"Session level or invalid attribute found"\
							" when expecting SDP media attribute");
					sip_error(SIP_Minor,sdpline);
					*(pSdpParam->pSdpParam->pError) = E_PARSER_ERROR;
					return;
				}
		}
	}
#ifdef SIP_SELECTIVE_PARSE
	if(*(pSdpParam->pSdpParam->pError) != E_NO_ERROR)
	{
		if(sip_acceptIncorrectSdpLine (sdpline) == SipSuccess)
				{
					/* Ignore the error */
					*(pSdpParam->pSdpParam->pError) = E_NO_ERROR;
					IncorrectSdpLine = STRDUPDECODE(sdpline);
					if(pSdpParam->parserState == 1)
					{
						if(sip_listAppend(&pSdpParam->pSdpParam->pMedia->\
									slIncorrectLines,IncorrectSdpLine,\
									pSdpParam->pSdpParam->pError)==SipFail)
						{
							if(IncorrectSdpLine != SIP_NULL)
								fast_memfree(DECODE_MEM_ID,IncorrectSdpLine,\
									&tempError);
							return ;
						}
					}
					else
					{
						if (sip_listAppend(&(pSdpParam->pSdpParam->\
								    pSdpMessage-> slIncorrectLines),\
									IncorrectSdpLine,pSdpParam->pSdpParam->\
									pError)==SipFail)
						{
							if(IncorrectSdpLine != SIP_NULL)
								fast_memfree(DECODE_MEM_ID,IncorrectSdpLine,&tempError);
							return ;
						}
					}
				}
	}
#endif
	if(sdpoutline != SIP_NULL)
		fast_memfree(DECODE_MEM_ID, sdpoutline, &tempError);
}


/*****************************************************************************
 ** FUNCTION: sip_checkMandatory
 *****************************************************************************
 ** Checks to see if message has mandatory some mandatory headers
 ** Currently checks for Call-Id, CSeq, From ant To headers so that the timer
 ** module that uses these doesnt throw up. Does not look for all the
 ** mandatory headers which differ depending on the message dType.
 ****************************************************************************/
SipBool sip_checkMandatory
#ifdef ANSI_PROTO
(SipMessage *s)
#else
(s)
	SipMessage *s;
#endif
{
	SIP_U32bit MissingHdrCount =0;

	if (s->dType == SipMessageRequest)
	{
		if((s->pGeneralHdr->pCallidHdr == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pCseqHdr == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pFromHeader == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pToHdr == SIP_NULL))
			MissingHdrCount++;
		if( (strcmp(s->u.pRequest->pRequestLine->pMethod,"PRACK")\
					==0) && (s->u.pRequest->pRequestHdr->pRackHdr == SIP_NULL) )
			MissingHdrCount++;
		if ( MissingHdrCount != 0 )
		{
			s->dIncorrectHdrsCount += MissingHdrCount;
			sip_error(SIP_Minor,\
					(SIP_S8bit *)"Message does not contain Mandatory Headers");
			return SipFail;
		}
	}
	else
	{
		if((s->pGeneralHdr->pCallidHdr == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pCseqHdr == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pFromHeader == SIP_NULL))
			MissingHdrCount++;
		if((s->pGeneralHdr->pToHdr == SIP_NULL))
			MissingHdrCount++;
		if ( MissingHdrCount != 0 )
		{
			s->dIncorrectHdrsCount += MissingHdrCount;
			sip_error(SIP_Minor,\
					(SIP_S8bit *)"Message does not contain Mandatory Headers");
			return SipFail;
		}
	}
	return SipSuccess;
}


/*****************************************************************************
 ** FUNCTION: sip_decodeMessage
 *****************************************************************************
 *  DESCRIPTION: Given in sipdecode.h and API reference
 ****************************************************************************/
#ifndef SIP_NO_CALLBACK
SipBool sip_decodeMessage
#ifdef ANSI_PROTO
(SIP_S8bit *message, SipOptions *opt, SIP_U32bit messageLength,\
 SIP_S8bit **nextmesg, SipEventContext *pContext, SipError *err)
#else
( message, opt, messageLength, nextmesg, pContext, err)
	SIP_S8bit *message;
	SipOptions *opt;
	SIP_U32bit messageLength;
	SIP_S8bit **nextmesg;
	SipEventContext *pContext;
	SipError *err;
#endif
#else
	SipBool sip_decodeMessage
#ifdef ANSI_PROTO
	(SIP_S8bit *message, SipMessage **ppDecodedMsg,\
	 SipOptions *opt, SIP_U32bit messageLength,\
	 SIP_S8bit **nextmesg, SipEventContext *pContext, SipError *err)
#else
( message, ppDecodedMsg, opt, messageLength, nextmesg, pContext, err)
	SIP_S8bit *message;
	SipMessage **ppDecodedMsg;
	SipOptions *opt;
	SIP_U32bit messageLength;
	SIP_S8bit **nextmesg;
	SipEventContext *pContext;
	SipError *err;
#endif
#endif
{
	SipMessage *tempMessage=SIP_NULL;
#ifdef SIP_TRACE
	SIP_S8bit *tracetemp;
	SipError traceError;
#endif
	INC_API_COUNT
#ifdef SIP_TRACE
		sip_trace(SIP_Brief,SIP_Incoming,(SIP_S8bit *)\
				"Message received for decoding");
	tracetemp = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
			(messageLength+2)*sizeof(SIP_S8bit),&traceError);
	if(tracetemp==SIP_NULL)
	{
		*(err)=E_NO_ERROR;
		return SipFail;
	}
	strncpy(tracetemp,message,messageLength);
	tracetemp[messageLength] = '\0';
	sip_trace(SIP_Detailed,SIP_Incoming,tracetemp);
	fast_memfree(DECODE_MEM_ID,tracetemp,SIP_NULL);
#endif
	*(err)=E_NO_ERROR;

	if(glbSipParserDecodeMessage(message,opt, messageLength,nextmesg,pContext,\
				&tempMessage, err) == SipFail)
	{
		switch(*err)
		{
			case E_INCOMPLETE:
#ifdef SIP_NO_CALLBACK
				/*This change is being made to ensure that even if there is an
				  error due to incomplete messages, the application can
				  at least access the message*/
				*ppDecodedMsg = tempMessage;
				if((opt->dOption & SIP_OPT_BADMESSAGE) != SIP_OPT_BADMESSAGE)
				{
					INC_ERROR_PROTOCOL
				}

				break;
#else
				sip_freeSipMessage(tempMessage);
				break;
#endif
			case E_MAYBE_INCOMPLETE:
			case E_PARSER_ERROR:
				{
					if((opt->dOption & SIP_OPT_BADMESSAGE) != SIP_OPT_BADMESSAGE)
					{
						INC_ERROR_PROTOCOL
					}
					break;
				}
			case E_NO_MEM:
				{
					INC_ERROR_MEM
						break;
				}
			default:;
		}
		return SipFail;
	}

	/* make sure trailing LWSs dont constitute a next message */
	if (*nextmesg !=SIP_NULL)
	{
		SIP_U32bit ln = 0;
		SIP_U32bit ws = 0;
		SIP_U32bit messglength = strlen(*nextmesg);

		while (ln < messglength)
		{
			if (isspace((int) ((*nextmesg)[ln]))) ws++;
			ln++;
		}
		/* release *nextmesg if its only LWS */
		if (ln == ws)
		{
			fast_memfree(DECODE_MEM_ID,*nextmesg, SIP_NULL);
			*nextmesg = SIP_NULL;
		}
	}

#ifndef SIP_TXN_LAYER
	if((opt->dOption & SIP_OPT_NOTIMER) != SIP_OPT_NOTIMER)
	{
		/* Invoke timer callbacks */
		if(sip_decodeInvokeTimerCallbacks(tempMessage, pContext,err)==SipFail)
		{
			/* Mandatory header check failed for message */
			if((opt->dOption & SIP_OPT_BADMESSAGE) == SIP_OPT_BADMESSAGE)
			{
				/* If BADMESSAGE option is set we ignore this error */
				DEC_ERROR_PROTOCOL
			}
			else
			{
				sip_freeSipMessage(tempMessage);
				return SipFail;
			}
		}
	}
#endif

	/* At this point,  we need to call the correct user callback
	   Callbacks are:
	   sip_indicateInvite()
	   sip_indicateCancel()
	   sip_indicateRegister()
	   sip_indicateAck()
	   sip_indicateBye()
	   sip_indicateOptions()
	   sip_indicateUnknownRequest()
	   sip_indicateRefer()
	   sip_indicateComet()
	   sip_indicateUpdate()

	   sip_indicateInformational()
	   sip_indicateFinalResponse()

NOTE: If the message could not be decoded, no callback is called.
Rather, the pUser is returned SipFail in
sip_decode()
	 */

	if(tempMessage->dType == SipMessageRequest)
	{
		SIP_S8bit *pMethod = tempMessage->u.pRequest->pRequestLine->pMethod;

		INC_API_REQ_PARSED
#ifdef SIP_MIB
			if ( __sip_incrementStats(pMethod,SIP_STAT_API_REQUEST,\
						SIP_STAT_IN) != SipSuccess )
			{
				INC_API_UNKNOWN_PARSED_IN
			}
#endif
		if(strcmp(pMethod,"INVITE")==0)
		{
			/* Call INVITE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateInvite(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"PUBLISH")==0)
		{
			/* Call PUBLISH indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicatePublish(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"ACK")==0)
		{
			/* Call ACK indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateAck(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"BYE")==0)
		{
			/* Call BYE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateBye(tempMessage, pContext);
#endif
		}

		else if(strcmp(pMethod,"REFER")==0)
		{
			/* Call REFER indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateRefer(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"REGISTER")==0)
		{
			/* Call REGISTER indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateRegister(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"OPTIONS")==0)
		{
			/* Call OPTIONS indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateOptions(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"CANCEL")==0)
		{
			/* Call CANCEL indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateCancel(tempMessage, pContext);
#endif
		}

#ifdef SIP_DCS
		else if(strcmp(pMethod,"COMET")==0)
		{
			/* Call COMET indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateComet(tempMessage, pContext);
#endif
			/********** to check whether timer stop is needed ***********/
		}
#endif
		else if(strcmp(pMethod,"INFO")==0)
		{
			/* Call INFO indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateInfo(tempMessage, pContext);
#endif
		}

		else if(strcmp(pMethod,"PROPOSE")==0)
		{
			/* Call PROPOSE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicatePropose(tempMessage, pContext);
#endif
		}

		else if(strcmp(pMethod,"PRACK")==0)
		{
			/* Call PRACK indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicatePrack(tempMessage, pContext);
#endif
		}
#ifdef SIP_IMPP
		else if(strcmp(pMethod,"SUBSCRIBE")==0)
		{
			/* Call SUBSCRIBE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateSubscribe(tempMessage, pContext);
#endif
		}

		else if(strcmp(pMethod,"NOTIFY")==0)
		{
			/* Call NOTIFY indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateNotify(tempMessage, pContext);
#endif
		}
		else if(strcmp(pMethod,"MESSAGE")==0)
		{
			/* Call MESSAGE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateMessage(tempMessage, pContext);
#endif
		}
#endif
		else if(strcmp(pMethod,"UPDATE")==0)
		{
			/* Call UPDATE indicate */
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateUpdate(tempMessage, pContext);
#endif
		}
		else
		{
#ifdef SIP_NO_CALLBACK
			*ppDecodedMsg = tempMessage;
#else
			sip_indicateUnknownRequest(tempMessage, pContext);
#endif
		}
	}
	else if(tempMessage->dType == SipMessageResponse)
	{
		SIP_U16bit codenum = tempMessage->u.pResponse->pStatusLine->dCodeNum;
#ifdef SIP_MIB
		if((codenum>=100)&&(codenum<200))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_1XX)
		}
		else if((codenum>=200)&&(codenum<300))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_2XX)
		}
		else if((codenum>=300)&&(codenum<400))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_3XX)
		}
		else if((codenum>=400)&&(codenum<500))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_4XX)
		}
		else if((codenum>=500)&&(codenum<600))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_5XX)
		}
		else if((codenum>=600)&&(codenum<700))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_6XX)
		}
		else if((codenum>=700)&&(codenum<800))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_7XX)
		}
		else if((codenum>=800)&&(codenum<900))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_8XX)
		}
		else if((codenum>=900)&&(codenum<1000))
		{
			INC_API_RESPCLASS_PARSED_IN(SIP_STAT_RESPCLASS_9XX)
		}

		if ( codenum < NUM_STAT_RESPONSECODE )
		{
			INC_API_RESPCODE_PARSED_IN(codenum)
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockStatisticsMutex,\
					FAST_MXLOCK_EXCLUSIVE );
#endif

			/*Did the application register for receiving callbacks??*/
			if (glbSipParserResponseCodeParserStats[codenum].dGiveCallback\
					==SipSuccess)
			{
				/*
				 * Throw the cbk indicating the sending of a response
				 * message of this type. Also add a reference to the
				 * actual message that caused this cbk to be invoked
				 */

				/*First increment the refcount of the SipMessage*/
				HSS_INCREF(tempMessage->dRefCount);

				sip_indicateResponseHandled(tempMessage, \
						SipCallbkForResponseRecvd, codenum,\
						glbSipParserResponseCodeParserStats[codenum]);
			}
#ifdef SIP_THREAD_SAFE
			fast_unlock_synch(0,&glbLockStatisticsMutex );
#endif
		}
#endif
		INC_API_RESP_PARSED
			if((codenum>=100)&&(codenum<200))
			{
#ifdef SIP_NO_CALLBACK
				*ppDecodedMsg = tempMessage;
#else
				sip_indicateInformational(tempMessage, pContext);
#endif
			}
			else
			{
#ifdef SIP_NO_CALLBACK
				*ppDecodedMsg = tempMessage;
#else
				sip_indicateFinalResponse(tempMessage, pContext);
#endif
			}
	}
	/* Now coming here means the user has been indicated of a message and
	   the necessary action has been taken.
	   Now, we take the (possible) next SIP Message and return it to him
	   after his sip_decode() */
	return SipSuccess;
}


#ifndef SIP_NO_CALLBACK
SipBool sip_decodeMessageWithoutCallback
#ifdef ANSI_PROTO
(SIP_S8bit *message, SipMessage **ppDecodedMsg,\
 SipOptions *opt, SIP_U32bit messageLength,\
 SIP_S8bit **nextmesg, SipEventContext *pContext, SipError *err)
#else
( message, ppDecodedMsg, opt, messageLength, nextmesg, pContext, err)
	SIP_S8bit *message;
	SipMessage **ppDecodedMsg;
	SipOptions *opt;
	SIP_U32bit messageLength;
	SIP_S8bit **nextmesg;
	SipEventContext *pContext;
	SipError *err;
#endif
{
	SipMessage *tempMessage=SIP_NULL;
#ifdef SIP_TRACE
	SIP_S8bit *tracetemp;
	SipError traceError;
#endif

	INC_API_COUNT

#ifdef SIP_TRACE
		sip_trace(SIP_Brief,SIP_Incoming,(SIP_S8bit *)\
				"Message received for decoding");
	tracetemp = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
			(messageLength+2)*sizeof(SIP_S8bit),&traceError);
	if(tracetemp==SIP_NULL)
	{
		*(err)=E_NO_ERROR;
		return SipFail;
	}
	strncpy(tracetemp,message,messageLength);
	tracetemp[messageLength] = '\0';
	sip_trace(SIP_Detailed,SIP_Incoming,tracetemp);
	fast_memfree(DECODE_MEM_ID,tracetemp,SIP_NULL);
#endif
	*(err)=E_NO_ERROR;
	/*sip_initSipMessage(tempMessage,SipMessageAny,err);*/
	if(glbSipParserDecodeMessage(message,opt, messageLength,nextmesg, \
				pContext,&tempMessage, err) == SipFail)
	{
		switch(*err)
		{
			case E_INCOMPLETE:
			case E_MAYBE_INCOMPLETE:
			case E_PARSER_ERROR:
				if((opt->dOption & SIP_OPT_BADMESSAGE) != SIP_OPT_BADMESSAGE)
				{
					INC_ERROR_PROTOCOL
				}
				break;
			case E_NO_MEM:
				INC_ERROR_MEM
					break;
			default:;
		}
		return SipFail;
	}

	/* make sure trailing LWSs dont constitute a next message */
	if (*nextmesg !=SIP_NULL)
	{
		SIP_U32bit ln = 0;
		SIP_U32bit ws = 0;
		SIP_U32bit messglength = strlen(*nextmesg);
		while (ln < messglength)
		{
			if (isspace((int) ((*nextmesg)[ln]))) ws++;
			ln++;
		}
		/* release *nextmesg if its only LWS */
		if (ln == ws)
		{
			fast_memfree(DECODE_MEM_ID,*nextmesg, SIP_NULL);
			*nextmesg = SIP_NULL;
		}
	}

	/* Now see if mandatory fields are persent : We now check for To, From,
	   Callid., CSEQ */
#ifndef SIP_TXN_LAYER
	if((opt->dOption & SIP_OPT_NOTIMER) != SIP_OPT_NOTIMER)
	{
		if(sip_decodeInvokeTimerCallbacks(tempMessage, pContext,err)\
				==SipFail)
		{
			/* Mandatory header check failed for message */
			if((opt->dOption & SIP_OPT_BADMESSAGE) == SIP_OPT_BADMESSAGE)
			{
				/* If BADMESSAGE option is set we ignore this error */
			}
			else
			{
				sip_freeSipMessage(tempMessage);
				return SipFail;
			}
		}
	}
	if(tempMessage->dType == SipMessageResponse)
	{
		INC_API_RESP_PARSED
	}
	else
	{
		INC_API_REQ_PARSED
	}
#endif
	*ppDecodedMsg = tempMessage;

	return SipSuccess;
}
#endif

#ifndef SIP_TXN_LAYER
SipBool sip_decodeInvokeTimerCallbacks
#ifdef ANSI_PROTO
(SipMessage *tempMessage, SipEventContext *pContext,SipError *err)
#else
(tempMessage, pContext,err)
	SipMessage *tempMessage;
	SipEventContext *pContext;
	SipError *err;
#endif
{
	if (sip_checkMandatory(tempMessage) == SipFail)
	{
		*err = E_PARSER_ERROR;
		INC_ERROR_PROTOCOL
			return SipFail;
	}

	if(tempMessage->dType == SipMessageRequest)
	{
		SIP_S8bit *pMethod = tempMessage->u.pRequest->pRequestLine->pMethod;

		if(strcmp(pMethod,"ACK")==0)
		{
			sip_stopTimer(tempMessage,pContext, err);
		}
		else if(strcmp(pMethod,"PRACK")==0)
		{
			sip_stopTimer(tempMessage,pContext, err);
		}
	}
	else if(tempMessage->dType == SipMessageResponse)
	{
		SIP_U16bit codenum=tempMessage->u.pResponse->pStatusLine->dCodeNum;

		if((codenum>=100)&&(codenum<200))
		{
			/* Form timerkey and restart timer with informational flag on */
			/* If INVITE, informational stops transmissions */
			if(strcmp(tempMessage->pGeneralHdr->pCseqHdr->pMethod,\
						"INVITE"))
				sip_updateTimer(tempMessage,SipSuccess,pContext,err);
			else
				sip_stopTimer(tempMessage,pContext,err);
		}
		else
		{
			/* Form timer pKey and pStop timer */
			/* Call Final indicate */
			sip_stopTimer(tempMessage,pContext,err);
		}
	}
	return SipSuccess;
}
#endif

/*****************************************************************************
 ** FUNCTION: glbSipParserMemStrStr
 *****************************************************************************
 ** Internal function for comparing a memory Buffer with a string
 ** The pBuffer may contain '\0'. Required to locate content boundaries in
 ** multipart messages which may contain binary encoded parts that contain
 ** '\0' which prevents usage of strstr.
 ****************************************************************************/

SipBool glbSipParserMemStrStr
#ifdef ANSI_PROTO
(SIP_S8bit *pBuffer,SIP_U32bit buflen, SIP_S8bit *separator, SIP_U32bit *offset)
#else
(pBuffer, buflen, separator, offset)
	SIP_S8bit *pBuffer;
	SIP_U32bit buflen;
	SIP_S8bit *separator;
	SIP_U32bit *offset;
#endif
{
	SIP_U32bit srcindex=0, cmpindex=0, cmplen;

	cmplen = strlen(separator);
	while(srcindex<buflen)
	{
		if(pBuffer[srcindex]==separator[cmpindex])
		{
			srcindex++;
			cmpindex++;
		}
		else
		{
			srcindex += 1-cmpindex;
			cmpindex = 0;
		}
		if(cmpindex==cmplen) break;
	}
	if(cmpindex!=cmplen)
		return SipFail;
	*offset = srcindex - cmpindex;
	return SipSuccess;
}


/*****************************************************************************
 ** FUNCTION: glbSipParserParseBody
 *****************************************************************************
 ** Function to parse message pBody. Function is passed the content-Type of
 ** the message Body, the Buffer with its Length and a siplist of MsgBody
 ** into which the parsed pBody is appended.
 ** The function calls itself recursively to deal with multipart messages
 ** within multipart.
 ****************************************************************************/
SipBool glbSipParserParseBody
#ifdef ANSI_PROTO
(SIP_U8bit dLevel, SipOptions *opt, SipContentTypeHeader *ctypehdr,\
 SIP_S8bit *messagebody,SIP_U32bit clen, SipList *msgbodylist, SipError *err)
#else
(dLevel, opt, ctypehdr, messagebody, clen, msgbodylist, err)
	SIP_U8bit dLevel;
	SipOptions *opt;
	SipContentTypeHeader *ctypehdr;
	SIP_S8bit *messagebody;
	SIP_U32bit clen;
	SipList *msgbodylist;
	SipError *err;
#endif
{
	SIP_U32bit bodystartindex = 0, contentendindex,i,j;
	SIP_U8bit sdpFlag;
	SipList sdpheaders;
	SIP_S8bit * pSipParserInput=SIP_NULL;
	SIP_U32bit dLoc=0;
	SIP_S8bit dBackChar=0;
	SipBool dMsgBodyTypeNotFound = SipFail;
	/*
	   messagebody
	   clen to be updated to correct Value,
	   bodystartindex to be set to beginning of newmessage.
	 */

	/* This function will have the value SIP_OPT_NOPARSEBODY in options
	   field option is set while calling sip_decode. Typically a proxy which
	   is not interested in body parsing will set this option. This
	   increases parsing speed */
	if ((opt->dOption & SIP_OPT_NOPARSEBODY)==SIP_OPT_NOPARSEBODY)
	{
		/* Body content Type is unknown */
		SipMsgBody *msgbody;
		SipUnknownMessage *unmsg;

		sip_trace(SIP_Brief,SIP_Incoming, (SIP_S8bit *)\
				"SIP_OPT_NOPARSEBODY set - skipping Body parse");
		if(sip_initSipMsgBody(&msgbody,SipUnknownBody,err)==SipFail)
			return SipFail;
		if(sip_initSipUnknownMessage(&unmsg, err) == SipFail)
			return SipFail;
		unmsg->pBuffer = (SIP_S8bit *) fast_memget\
			(DECODE_MEM_ID,clen+1,err);
		if(unmsg->pBuffer==SIP_NULL)
		{
			*(err)= E_NO_MEM;
			return SipFail;
		}
		memcpy(unmsg->pBuffer, &messagebody[bodystartindex], clen);
		unmsg->pBuffer[clen]='\0';
		unmsg->dLength = clen;
		msgbody->u.pUnknownMessage = unmsg;
		if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
			return SipFail;
		*err = E_NO_ERROR;
		return SipSuccess;

	}
	/* Message Body exists. Now check if it is an sdp message */
	sdpFlag=0; /* changed from 1. Now SDP parsed only if clen specified */
	if ( ctypehdr != SIP_NULL )
	{
		if  ( ((strcasecmp(ctypehdr->pMediaType,"application/sdp")==0) ) && \
				((opt->dOption & SIP_OPT_NOPARSESDP)!=SIP_OPT_NOPARSESDP) )
		{
			/* Message pBody contains SDP message */
			SipError dErr;
			SipSdpParserParam dSdpParserParam;
			SipParseEachSdpLineParam dSdpParseEachParam;
			dSdpParserParam.pError=(SipError *)(fast_memget(DECODE_MEM_ID,\
						sizeof(SipError),err));
			if(dSdpParserParam.pError==SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			*(dSdpParserParam.pError)=E_NO_ERROR;
			dSdpParserParam.pGCList=(SipList*)fast_memget(DECODE_MEM_ID,\
					sizeof(SipList),err);
			if(dSdpParserParam.pGCList==SIP_NULL)
			{
				fast_memfree(DECODE_MEM_ID,dSdpParserParam.pError,SIP_NULL);
				*err = E_NO_MEM;
				return SipFail;
			}
			sip_listInit((dSdpParserParam.pGCList),&sip_freeVoid,err);
			if(*err !=E_NO_ERROR)
			{
				fast_memfree(DECODE_MEM_ID,dSdpParserParam.pError,SIP_NULL);
				fast_memfree(DECODE_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
				return SipFail;
			}
			contentendindex = clen;
			/* content dLength and pStart of pBody found */
			/* Now ignore leading whitespace */
			while(isspace((int)messagebody[bodystartindex])&&\
					(bodystartindex<contentendindex))
				bodystartindex++;

			if (clen!=0)
			{
				SipMsgBody *msgbody;
				int flag = 0;

				if(bodystartindex>=(contentendindex))
				{
					/* SDP message with whitespace */
					/* set pBody to null and exit ? */
					fast_memfree(DECODE_MEM_ID,dSdpParserParam.pError,SIP_NULL);
					fast_memfree(DECODE_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
					return SipFail;
				}
				/* Initialize structures for the sdp parser */
				dSdpParserParam.pSdpMessage = (SdpMessage *)\
					fast_memget(DECODE_MEM_ID,sizeof(SdpMessage),err);
				if(dSdpParserParam.pSdpMessage == SIP_NULL)
				{
					fast_memfree(DECODE_MEM_ID,dSdpParserParam.pError,SIP_NULL);
					fast_memfree(DECODE_MEM_ID,dSdpParserParam.pGCList,SIP_NULL);
					*err=E_NO_MEM;
					return SipFail;
				}
				dSdpParserParam.pMedia = SIP_NULL;
				HSS_INITREF(dSdpParserParam.pSdpMessage->dRefCount);
				dSdpParserParam.pSdpMessage->pVersion = SIP_NULL;
				dSdpParserParam.pSdpMessage->pOrigin = SIP_NULL;
				dSdpParserParam.pSdpMessage->pSession = SIP_NULL;
				dSdpParserParam.pSdpMessage->pInformation = SIP_NULL;
				dSdpParserParam.pSdpMessage->pUri = SIP_NULL;
				dSdpParserParam.pSdpMessage->slConnection = SIP_NULL;
				dSdpParserParam.pSdpMessage->pKey = SIP_NULL;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->slEmail, \
							__sip_freeString, err)==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->slPhone, \
							__sip_freeString, err)==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.\
							pSdpMessage->pBandwidth,\
							__sip_freeString,err) ==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->slTime, \
							__sip_freeSdpTime, err)==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->slAttr,\
							__sip_freeSdpAttr,err)==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->slMedia ,\
							__sip_freeSdpMedia,err)==SipFail)
					return SipFail;
				if(sip_listInit(&dSdpParserParam.pSdpMessage->\
							slIncorrectLines,__sip_freeString, err)==\
						SipFail)
					return SipFail;
				if(sip_listInit(&sdpheaders,&sip_freeVoid,err)==SipFail)
					return SipFail;
				i=j=bodystartindex;
				/* Break up sdp Body into lines */
				/* Lines stored in list sdpheaders of char* */

				/* Now remove all trailing LWS' from SDP Message */

				contentendindex--;
				while (isspace((int)\
							messagebody[contentendindex]) || \
						(messagebody[contentendindex]=='\0'))
				{
					contentendindex--;
					flag = 1;
				}
				if (flag == 1)
				{
					/*
					   Save the character that would get
					   overwritten during the stripping off
					   of the whitespace
					 */
					dLoc = contentendindex+1;
					dBackChar = messagebody[dLoc];
					messagebody[contentendindex+1]='\0';
					contentendindex+=1;
				}

				while(i<=(contentendindex))
				{
					if((messagebody[i]=='\r')||(messagebody[i]=='\n')||\
							(messagebody[i]=='\0'))
					{
						SIP_S8bit *temp;

						temp = (SIP_S8bit *) \
							fast_memget(DECODE_MEM_ID,\
									sizeof(SIP_S8bit)*(i-j+2),err);
						if(temp==SIP_NULL)
							return SipFail;
						strncpy(temp,&messagebody[j],i-j);
						temp[i-j] = '\0';
						temp[i-j+1] = '\0';
						if(sip_listAppend(&sdpheaders,temp,err)\
								==SipFail)
							return SipFail;
						i++;
						j=i;
						if(i>=(contentendindex))
						{
							/* Reached end of Body
							   break out before next check */
							break;
						}
						if((messagebody[i-1]=='\r')\
								&&(messagebody[i]=='\n'))
						{
							i++;
							j++;
						}
					}
					else
					{
						i++;
						if(i>=contentendindex)
						{
							SIP_S8bit *temp;
							temp = (SIP_S8bit *) \
								fast_memget(DECODE_MEM_ID,\
										sizeof(SIP_S8bit)*(i-j+3),err);
							if(temp==SIP_NULL)
								return SipFail;
							strncpy(temp,&messagebody[j],i-j+1);
							temp[i-j+1] = '\0';
							temp[i-j+2] = '\0';
							if(sip_listAppend\
									(&sdpheaders,temp,err)==SipFail)
								return SipFail;
							break;
						}
					}
				}

				if (flag==1)
				{
					/*Put back the character*/
					messagebody[dLoc] = dBackChar;
				}

				/* glbSdpParser state used to find if lines being
				   parsed are part of a slMedia description
				   state = 0 ==> parsing line for global description
				   state = 1 ==> parsing line for slMedia */
				/* Parse each line in list */
				*(dSdpParserParam.pError) = E_NO_ERROR;
				dSdpParseEachParam.pSdpParam = &dSdpParserParam;
				dSdpParseEachParam.parserState = 0;
				dSdpParseEachParam.repeatState = 0;
				sip_listForEachWithData(&sdpheaders,\
						&glbSipParserSdpHeaderParse,\
						(SIP_Pvoid *)&dSdpParseEachParam,err);
				/*free all resources collected by GC in parsing phase */
				if(sip_listDeleteAll\
						((dSdpParserParam.pGCList),&dErr)==SipFail)
				{
					return SipFail;
				}
				if(*(dSdpParserParam.pError) != E_NO_ERROR)
				{
					sip_listDeleteAll(&sdpheaders,&dErr);
					/* sip_freeSdpMedia(dSdpParserParam.pMedia); */
					sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
					*err = *(dSdpParserParam.pError);
					fast_memfree\
						(DECODE_MEM_ID,dSdpParserParam.pError,SIP_NULL);
					fast_memfree(DECODE_MEM_ID,\
							dSdpParserParam.pGCList,SIP_NULL);
					return SipFail;
				}

				if(sip_initSipMsgBody(&msgbody,SipSdpBody,err)==SipFail)
					return SipFail;
				msgbody->u.pSdpMessage = dSdpParserParam.pSdpMessage;
				if(sip_listAppend\
						(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
					return SipFail;
				if(sip_listDeleteAll(&sdpheaders, err)==SipFail)
					return SipFail;
			} /* of if clen != 0 */
			fast_memfree\
				(DECODE_MEM_ID,dSdpParserParam.pGCList,err);
			fast_memfree(DECODE_MEM_ID,dSdpParserParam.\
					pError,err);
		}
	#ifdef SIP_MWI
		else if(strcasecmp\
				(ctypehdr->pMediaType,"application/simple-message-summary") == 0)
		{
			SipMsgBody *msgbody;
			MesgSummaryMessage *pSummaryMessage;
			SipMesgSummaryParserParam dMesgSummaryParserParam;
			SIP_S8bit* pMwiBuffer;
			SipError error;
			SIP_S8bit savedChar[2];

			if(sip_initSipMsgBody(&msgbody,SipMessageSummaryBody,err) == \
					SipFail)
				return SipFail;
			if(sip_mwi_initMesgSummaryMessage(&pSummaryMessage, err) == SipFail)
				return SipFail;

			dMesgSummaryParserParam.pError = &error;
			*(dMesgSummaryParserParam.pError) = E_NO_ERROR;
			dMesgSummaryParserParam.pMesgSummaryMessage = pSummaryMessage;


			if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == SIP_OPT_DBLNULLBUFFER)
			{
				pMwiBuffer = &messagebody[bodystartindex];
				/* save characters parser needs two zero for end of input*/
				savedChar[0] = pMwiBuffer[clen+1];
				savedChar[1] = pMwiBuffer[clen+2];
				pMwiBuffer[clen] = '\0';
				pMwiBuffer[clen+1] = '\0';
			}
			else
			{
				/* allocate new buffer */
				pMwiBuffer = (SIP_S8bit *)fast_memget(DECODE_MEM_ID,clen+2,err);
				strncpy(pMwiBuffer,&messagebody[bodystartindex],clen+2);
				pMwiBuffer[clen] = '\0';
				pMwiBuffer[clen+1] = '\0';
			}
			/* set the scan buffer to lex */
			if(sip_lex_MesgSummary_scan_buffer(pMwiBuffer, \
						clen+2) != 0)
			{
				fast_memfree(DECODE_MEM_ID,pMwiBuffer,err);
				sip_mwi_freeMesgSummaryMessage(pSummaryMessage);
				sip_freeSipMsgBody(msgbody);
				return SipFail;
			}
			/* parse the msg body */
			sip_lex_MesgSummary_reset_state();
			glbSipParserMesgSummaryparse((void *)&dMesgSummaryParserParam);
			sip_lex_MesgSummary_release_buffer();

			if(((opt->dOption & SIP_OPT_DBLNULLBUFFER) == SIP_OPT_DBLNULLBUFFER))
			{
				/* revert back the orginal chars from saved ones */
				pMwiBuffer[clen] = savedChar[0];
				pMwiBuffer[clen+1] = savedChar[1];
			}
			else
			{
				/* free the buffer allocated */
				fast_memfree(DECODE_MEM_ID,pMwiBuffer,err);
			}
			/* if there is a parse error */
			if(*(dMesgSummaryParserParam.pError) != E_NO_ERROR)
			{
				sip_mwi_freeMesgSummaryMessage(pSummaryMessage);
				sip_freeSipMsgBody(msgbody);
				return SipFail;
			}
			/* assign the parsed message to msgbody list */
			msgbody->u.pSummaryMessage = pSummaryMessage;
			if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
				return SipFail;

		}
	#endif
		else if(strcasecmp(ctypehdr->pMediaType,"application/ISUP")==0)
		{
			/* Body content dType is ISUP */
			SipMsgBody *msgbody;
			IsupMessage *isupmsg;

			if(sip_initSipMsgBody(&msgbody,SipIsupBody,err)==SipFail)
				return SipFail;
			if(sip_bcpt_initIsupMessage(&isupmsg, err) == SipFail)
				return SipFail;
			isupmsg->pBody = (SIP_S8bit *) fast_memget\
				(DECODE_MEM_ID,clen+1,err);
			if(isupmsg->pBody==SIP_NULL)
			{
				*err=E_NO_MEM;
				return SipFail;
			}
			memcpy(isupmsg->pBody, &messagebody[bodystartindex], clen);
			isupmsg->pBody[clen]='\0';
			isupmsg->dLength = clen;
			msgbody->u.pIsupMessage = isupmsg;
			if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
				return SipFail;
		}
		else if(strcasecmp(ctypehdr->pMediaType,"application/QSIG")==0)
		{
			/* Body content dType is QSIG */
			SipMsgBody *msgbody=SIP_NULL;
			QsigMessage *qsigmsg=SIP_NULL;

			if(sip_initSipMsgBody(&msgbody,SipQsigBody,err)==SipFail)
				return SipFail;
			if(sip_bcpt_initQsigMessage(&qsigmsg, err) == SipFail)
				return SipFail;
			qsigmsg->pBody = (SIP_S8bit *) fast_memget(DECODE_MEM_ID, \
					clen+1,err);
			if(qsigmsg->pBody==SIP_NULL)
			{
				*err=E_NO_MEM;
				return SipFail;
			}
			
			/* CVS  1-1427584  **requested by NexTone** */
			memcpy(qsigmsg->pBody, &messagebody[bodystartindex],clen);
			qsigmsg->pBody[clen]='\0';
			qsigmsg->dLength = clen;
			msgbody->u.pQsigMessage = qsigmsg;
			if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
				return SipFail;
		}
		else if (strcasecmp(ctypehdr->pMediaType,"multipart/mixed")==0)
		{
			SIP_S8bit *separator,*boundary;
			SIP_U32bit offset,lastoffset, boundarylen;
			SIP_U32bit count, iterator, *tempnum;
			SipList separatoroffsets;
			SIP_U32bit endseparatoroffset;
			SipList *newmsgbodylist;
			SipMsgBody *multipartbody;

			/* Multipart mixed MIME message Body */
			/* Get boundary from content-Type Header */
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
								err)==SipFail) return SipFail;
					break;
				}
				iterator++;
			}
			if(boundary==SIP_NULL)
			{
				*err = E_PARSER_ERROR;
				sip_error\
					(SIP_Minor, "No boundary specified in multipart/mixed\n");
				return SipFail;
			}
			/* Split message Body into constituent elements */
			separator = (SIP_S8bit *)fast_memget\
				(DECODE_MEM_ID,sizeof(SIP_S8bit)*strlen(boundary)+3,err);
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
			/* Now find first occurance of separator*/
			/* String functions cannot be used since \0 might be present
			   in the pBuffer */
			lastoffset = 0;
			while(1)
			{
				SIP_U8bit bfound=0;
				if(glbSipParserMemStrStr\
						(&messagebody[lastoffset],clen-lastoffset,separator,\
						 &offset)==SipFail)
				{
					/* Multipart message without any separated messages */
					*err = E_PARSER_ERROR;
					fast_memfree(DECODE_MEM_ID,separator,SIP_NULL);
					sip_error\
						(SIP_Minor, "No boundary found in message body\n");
					return SipFail;
				}
				if(offset>=2)
				{
					if(!((messagebody[offset-2]=='\r')&&\
								(messagebody[offset-1]=='\n')))
					{
						lastoffset+=offset+boundarylen;
						break;
					}
				}
				lastoffset+=offset+boundarylen+2;
				while(lastoffset<clen-1)
				{
					if(isspace((int)messagebody[lastoffset]))
					{
						if((messagebody[lastoffset]=='\r')&&\
								(messagebody[lastoffset+1]=='\n'))
						{
							bfound = 1;
							lastoffset+=2;
							break;
						}
						else
							lastoffset++;
					}
					else
					{
						break;
					}
				}
				if(bfound) break;
			}
			if(sip_listInit(&separatoroffsets,__sip_freeString,err)==SipFail)
				return SipFail;
			tempnum = (SIP_U32bit *)\
				fast_memget(DECODE_MEM_ID,sizeof(SIP_U32bit),err);
			if(tempnum == SIP_NULL)
				return SipFail;
			*tempnum = lastoffset;
			if(sip_listAppend(&separatoroffsets,(SIP_Pvoid)tempnum,err)\
					==SipFail)
				return SipFail;
			endseparatoroffset = clen;
			/* First delimiter found */
			/* now get chunks */
			while(glbSipParserMemStrStr(&(messagebody[lastoffset]),\
						clen-lastoffset, separator,&offset)!=SipFail)
			{
				SIP_U8bit bfound=0;
				SIP_U8bit endboundary = 0;
				/* check for preceding CRLF */
				if(!((messagebody[lastoffset+offset-2]=='\r')\
							&&(messagebody[lastoffset+offset-1]=='\n')))
				{
					lastoffset+=offset+boundarylen;
					break;
				}
				/* check if the delimiter is the end-point delimiter */
				if(lastoffset+offset+boundarylen+3 < clen)
				{
					if((messagebody[lastoffset+offset+boundarylen+2]=='-')&&\
							(messagebody[lastoffset+offset+boundarylen+3]=='-'))
					{
						endboundary = 1;
						lastoffset+=2;
					}
				}
				/* now check for terminating CRLF */
				lastoffset+=offset+boundarylen+2;
				while(lastoffset<clen-1)
				{
					if(isspace((int)messagebody[lastoffset]))
					{
						if((messagebody[lastoffset]=='\r')&&\
								(messagebody[lastoffset+1]=='\n'))
						{
							bfound = 1;
							lastoffset+=2;
							break;
						}
						else
							lastoffset++;
					}
					else
					{
						break;
					}
				}
				if(bfound)
				{
					if(endboundary)
					{
						endseparatoroffset = lastoffset-boundarylen-6;
					}
					else
					{
						tempnum = (SIP_U32bit *)fast_memget\
							(DECODE_MEM_ID,sizeof(SIP_U32bit),err);
						if(tempnum == SIP_NULL)
							return SipFail;
						*tempnum = lastoffset;
						if(sip_listAppend(&separatoroffsets,\
									(SIP_Pvoid)tempnum,err)==SipFail)
							return SipFail;
					}
				}
			}

			/* Now separatoroffsets contains a list of Body offsets */
			/* endseparatoroffset contains the end of the last Body */
			if(sip_listSizeOf(&separatoroffsets,&count,err)==SipFail)
				return SipFail;
			/* About to process chunks */
			/* Set the list in which the chunks go in to */
			/* If Level 0, all chunks go into the list passed */
			/* else create a MsgBodynode n add chunks to the list inside it */
			if(dLevel==0)
			{
				newmsgbodylist = msgbodylist;
			}
			else
			{
				if(sip_initSipMsgBody(&multipartbody,SipMultipartMimeBody,err)\
						==SipFail)
					return SipFail;
				if(sip_bcpt_initMimeMessage\
						(&(multipartbody->u.pMimeMessage),err)==SipFail)
					return SipFail;
				newmsgbodylist = \
					&(multipartbody->u.pMimeMessage->slRecmimeBody);
				if(sip_listAppend(msgbodylist,(SIP_Pvoid)multipartbody,err)\
						==SipFail)
					return SipFail;
			}
			iterator = 0;
			while(iterator<count)
			{
				SIP_U32bit *jn,*k,jj,kk;
				if(sip_listGetAt(&separatoroffsets,iterator,(SIP_Pvoid*) &jn,\
							err) ==SipFail)
					return SipFail;
				jj=*jn;
				if(iterator!=(count-1))
				{
					if(sip_listGetAt(&separatoroffsets,iterator+1,\
								(SIP_Pvoid*) &k,err)==SipFail)
						return SipFail;
					kk = *k;
					kk-=boundarylen+4;
				}
				else
					kk=endseparatoroffset;
				/* Message Body part from offset jj to kk */
				if((messagebody[jj]=='\r')&&(messagebody[jj+1]=='\n'))
				{
					/* No MIME headers for this part */
					/* Body Type defaults to Text/plain */
					/* Stuff into unknown Body */
					/* Body content Type is unknown */
					SipMsgBody *msgbody;
					SipUnknownMessage *unmsg;
					SIP_U32bit ctlen;

					if(sip_initSipMsgBody(&msgbody,SipUnknownBody,err)==SipFail)
						return SipFail;
					if(sip_initSipUnknownMessage(&unmsg, err) == SipFail)
						return SipFail;
					ctlen = kk-jj-3;
					unmsg->pBuffer = (SIP_S8bit *) \
						fast_memget(DECODE_MEM_ID,ctlen+1,err);
					if(unmsg->pBuffer==SIP_NULL)
					{
						*err = E_NO_MEM;
						return SipFail;
					}
					memcpy(unmsg->pBuffer, &messagebody[jj+2], ctlen);
					unmsg->pBuffer[ctlen]='\0';
					unmsg->dLength = ctlen;
					msgbody->u.pUnknownMessage = unmsg;
					if(sip_listAppend(newmsgbodylist,(SIP_Pvoid)msgbody,err)\
							==SipFail)
						return SipFail;
				}
				else
				{
					SipMimeParserParam dMimeParserParam;
					SipError mimeError;
					SipMimeHeader *savedheader;
					SIP_U32bit bodylistcount;
					SipMsgBody *lastbody;
					SIP_S8bit *pMimeParserBuffer;
					SIP_S8bit mimeSavedChars[2];
					SipBool		isBodyPresent = SipSuccess;
					SipMsgBody *msgbody;

					/* MIME part headers present */
					/* Prepare Buffer for parser */
					SIP_U32bit bodyoffset,inputlimnum;

					if(glbSipParserMemStrStr(&messagebody[jj],(kk-jj),\
								(SIP_S8bit *)"\r\n\r\n",&bodyoffset)==SipFail)
					{
						bodyoffset = (kk-jj)-1;
						isBodyPresent = SipFail;
						/*sip_listDeleteAll(&separatoroffsets,err);
						sip_error (SIP_Minor, "Message part in multipart"\
								"with mime headers but no body\n");
						*err = E_PARSER_ERROR;
						return SipFail;*/
					}
					inputlimnum = bodyoffset+4;
					bodyoffset+=4+jj;

					/* Initialize global MimeHeader structure */
					if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == \
							SIP_OPT_DBLNULLBUFFER)
					{
						pMimeParserBuffer = &messagebody[jj];
						mimeSavedChars[0] = pMimeParserBuffer[inputlimnum];
						mimeSavedChars[1] = pMimeParserBuffer[inputlimnum+1];
						pMimeParserBuffer[inputlimnum] = '\0';
						pMimeParserBuffer[inputlimnum+1] = '\0';
					}
					else
					{
						pMimeParserBuffer = (SIP_S8bit *) \
							fast_memget(DECODE_MEM_ID, inputlimnum+2, err);
						if(pMimeParserBuffer == SIP_NULL)
							return SipFail;
						strncpy(pMimeParserBuffer, &(messagebody[jj]),\
								inputlimnum);
						pMimeParserBuffer[inputlimnum] = '\0';
						pMimeParserBuffer[inputlimnum+1] = '\0';
					}

					(dMimeParserParam.pError)=&mimeError;
					(*(dMimeParserParam.pError))=E_NO_ERROR;
					(dMimeParserParam.pGCList)=(SipList*)fast_memget(\
																	 DECODE_MEM_ID,sizeof(SipList),err);
					if(sip_listInit((dMimeParserParam.pGCList),\
								sip_freeVoid, err)==SipFail)
						return SipFail;
					if(sip_bcpt_initSipMimeHeader(&(dMimeParserParam.\
									pMimeHeader),err)==SipFail)
						return SipFail;
					*(dMimeParserParam.pError) = E_NO_ERROR;
					/*Here we remove any linefolds that might be present
					  in the MimeHdrs*/
					removeLineFold(pMimeParserBuffer);
					if (sip_lex_Mime_scan_buffer(pMimeParserBuffer, \
								inputlimnum+2) != 0)
					{
						sip_listDeleteAll(&separatoroffsets,err);
						fast_memfree(DECODE_MEM_ID,pMimeParserBuffer,SIP_NULL);
						sip_bcpt_freeSipMimeHeader\
							(dMimeParserParam.pMimeHeader);
						sip_listDeleteAll((dMimeParserParam.pGCList), err);
						fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
								dMimeParserParam.pGCList,err);
						*(dMimeParserParam.pError) = E_NO_MEM;
						return SipFail;
					}
					sip_lex_Mime_reset_state();
					pSipParserInput = pMimeParserBuffer;
					glbSipParserMimeparse((void *)&dMimeParserParam);
					sip_lex_Mime_release_buffer();
					if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == \
							SIP_OPT_DBLNULLBUFFER)
					{
						pMimeParserBuffer[inputlimnum] = mimeSavedChars[0];
						pMimeParserBuffer[inputlimnum+1] = mimeSavedChars[1];
					}
					else
					{
						fast_memfree(DECODE_MEM_ID,pMimeParserBuffer,SIP_NULL);
					}
					/* free all resources collected by GC in parsing phase */
					if(sip_listDeleteAll\
							((dMimeParserParam.pGCList), err)==SipFail)
					{
						return SipFail;
					}
					if(*(dMimeParserParam.pError) != E_NO_ERROR)
					{
						sip_listDeleteAll(&separatoroffsets,err);
						*err =*(dMimeParserParam.pError);
						sip_bcpt_freeSipMimeHeader\
							(dMimeParserParam.pMimeHeader);
						fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
								dMimeParserParam.pGCList,err);
						return SipFail;
					}
					/* glbSipParserMimeHeader now contains the MIME headers */
					/* MIME headers parsed. clone and save into a structure */
					/* Parse Body by invoking the this function recursively */
					/* Insert the save mime pHeader into the last element of*/
					/* the siplist when the function returns */
					/* save the reference in savedheader*/
					savedheader = dMimeParserParam.pMimeHeader;
					dMimeParserParam.pMimeHeader = SIP_NULL;

					if ( isBodyPresent == SipSuccess )
					{
						if ( bodyoffset >= (kk - 2) )
						{
							sip_bcpt_freeSipMimeHeader(savedheader);
							fast_memfree(DECODE_MEM_ID,\
									(SIP_Pvoid *)dMimeParserParam.pGCList,err);
							sip_listDeleteAll(&separatoroffsets,err);
							return SipFail;
						}
						if(glbSipParserParseBody(1,opt, savedheader->pContentType,\
									&messagebody[bodyoffset],kk-bodyoffset-2,\
									newmsgbodylist,err)==SipFail)
						{
							sip_bcpt_freeSipMimeHeader(savedheader);
							fast_memfree(DECODE_MEM_ID,\
									(SIP_Pvoid *)dMimeParserParam.pGCList,err);
							sip_listDeleteAll(&separatoroffsets,err);
							return SipFail;
						}
					}
					else
					{
						if(sip_initSipMsgBody(&msgbody,SipUnknownBody,err)\
							==SipFail)
							return SipFail;
						if(sip_listAppend(newmsgbodylist,\
							(SIP_Pvoid)msgbody,err)==SipFail)
							return SipFail;
					}
						
					/* Now add the saved MIME pHeader to the last element
					   in the list of MsgBody */
					if(sip_listSizeOf(newmsgbodylist,&bodylistcount,err)\
							==SipFail)
						return SipFail;
					
					if(sip_listGetAt(newmsgbodylist,bodylistcount-1,\
								(SIP_Pvoid*)&lastbody,err)==SipFail)
						return SipFail;
					/* save the reference in lastbody */
					lastbody->pMimeHeader = savedheader;
					savedheader = SIP_NULL;
					fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dMimeParserParam. \
							pGCList,err);
				}
				iterator++;
			}
			sip_listDeleteAll(&separatoroffsets,err);
			fast_memfree(DECODE_MEM_ID,separator,err);
		}
		else if ((strcasecmp(ctypehdr->pMediaType,"message/sipfrag")==0)  && \
				((opt->dOption & SIP_OPT_NOPARSEAPPSIP)!=SIP_OPT_NOPARSEAPPSIP))
		{
			/* Message body contains a SIP message */
			/* messagebody[bodystartindex] contains the first octet
			   clen is the length of the message body */
			SipMsgBody *msgbody;
			SipMessage *pSipMessage;
			SipOptions entityOpt;
			SIP_S8bit *pNextMesg;

			if(sip_initSipMsgBody(&msgbody,SipAppSipBody,err)==SipFail)
				return SipFail;

			entityOpt.dOption = SIP_OPT_NOTIMER | SIP_OPT_PARTIAL;
	#ifdef SIP_NO_CALLBACK
			if(sip_decodeMessage(&(messagebody[bodystartindex]),&pSipMessage,\
						&entityOpt, clen, &pNextMesg, SIP_NULL, err) == SipFail)
			{
				sip_freeSipMsgBody(msgbody);
				/* Decrement - else it'll indicate two errors */
				DEC_ERROR_PROTOCOL
					return SipFail;
			}
	#else
			if(sip_decodeMessageWithoutCallback(&(messagebody[bodystartindex]),\
						&pSipMessage, &entityOpt, clen, &pNextMesg, SIP_NULL, err)\
					== SipFail)
			{
				sip_freeSipMsgBody(msgbody);
				/* Decrement - else it'll indicate two errors */
				DEC_ERROR_PROTOCOL
					return SipFail;
			}
	#endif
			DEC_API_COUNT
				if(pSipMessage->dType == SipMessageResponse)
				{
					DEC_API_RESP_PARSED
				}
				else
				{
					DEC_API_REQ_PARSED
				}
			msgbody->u.pAppSipMessage = pSipMessage;
			if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
			{
				sip_freeSipMsgBody(msgbody);
				return SipFail;
			}
		}
		else
		{
			dMsgBodyTypeNotFound = SipSuccess;
		}
	}

	if((ctypehdr == NULL) || (dMsgBodyTypeNotFound == SipSuccess))
	{
		/* Body content Type is unknown */
		SipMsgBody *msgbody;
		SipUnknownMessage *unmsg;

		if(sip_initSipMsgBody(&msgbody,SipUnknownBody,err)==SipFail)
			return SipFail;
		if(sip_initSipUnknownMessage(&unmsg, err) == SipFail)
			return SipFail;
		unmsg->pBuffer = \
			(SIP_S8bit *) fast_memget(DECODE_MEM_ID,clen+1,err);
		if(unmsg->pBuffer==SIP_NULL)
		{
			*err=E_NO_MEM;
			return SipFail;
		}
		memcpy(unmsg->pBuffer, &messagebody[bodystartindex], clen);
		unmsg->pBuffer[clen]='\0';
		unmsg->dLength = clen;
		msgbody->u.pUnknownMessage = unmsg;
		if(sip_listAppend(msgbodylist,(SIP_Pvoid)msgbody,err)==SipFail)
			return SipFail;
	}
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserDecodeMessage
 *****************************************************************************
 ** Parses the message passed and fills up a global SipMessage structure.
 ** If the message contains multiple requests, the next request is returned
 ** in nextmesg.
 ****************************************************************************/
SipBool glbSipParserDecodeMessage
#ifdef ANSI_PROTO
(SIP_S8bit *message, SipOptions *opt, SIP_U32bit messageLength,\
 SIP_S8bit **nextmesg, SipEventContext *pContext, SipMessage **ppSipMessage,\
 SipError *err)
#else
( message, opt, messageLength, nextmesg, pContext, ppSipMessage,err)
	SIP_S8bit *message;
	SipOptions *opt;
	SIP_U32bit messageLength;
	SIP_S8bit **nextmesg;
	SipEventContext *pContext;
	SipMessage **ppSipMessage;
	SipError *err;
#endif
{
	SipParseEachHeaderParam dParseEachHeaderParam;
	SipHeaderParserParam dParserParams;
	SipUnknownHeader dHeader;
	SIP_S8bit reqSavedChars[2];
	SIP_S8bit headerSavedChars[2];
	SIP_S8bit *headerParserBuffer=SIP_NULL;
	SIP_S8bit *pReqLineParserBuffer=SIP_NULL;
	SIP_S8bit *encoutbuffer = SIP_NULL;
	SIP_S8bit *pSipParserInput=SIP_NULL;
	SIP_U32bit headerstartindex=0, headerendindex=0;
	SIP_U32bit scanLength = 0;
	SIP_U32bit lineEndIndex = 0;
	SIP_U32bit bodystartindex=0;

	SIP_U32bit messageBodyFlag;
	SIP_U32bit myinputindex,linestartindex;
	SIP_U32bit contentendindex;
	SIP_U32bit i=0,j=0;
	SIP_U32bit clen=0; /* to remove the warning while compialtion */
	SIP_S8bit singleLineMessage = 0;
	SipError dErr;
	SipBool dRetFromGetHdrLine=SipFail;


	*nextmesg = SIP_NULL;
	/* Parser Input Buffer initialization */
	myinputindex = messageLength ;
	*err = E_NO_ERROR;
	dHeader.pName=SIP_NULL;
	dHeader.pBody=SIP_NULL;
	/* init the garbage collector for each decodeMessage */
	dParserParams.pGCList=(SipList*)fast_memget(DECODE_MEM_ID,\
			sizeof(SipList),err);
	if(dParserParams.pGCList==SIP_NULL)
	{
		*err=E_NO_MEM;
		return SipFail;
	}

	if(sip_listInit((dParserParams.pGCList), sip_freeVoid, err)==\
			SipFail)
	{
		fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *) \
				dParserParams.pGCList,err);
		*err=E_NO_MEM;
		return SipFail;
	}

	if(messageLength == 0)
	{
		fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
				dParserParams.pGCList,err);
		*err = E_PARSER_ERROR;
		SIPDEBUG(\
				"SIP DECODE MESSAGE No Text in message. Returning fail\n");
		return SipFail;
	}

	/* Get to beginning response/request line */
	for(i=0;(isspace((int)message[i]))&&(i<myinputindex-1);i++) ;
	if(i==myinputindex)
	{
		/* message contains no Text */
		fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,err);
		*err = E_PARSER_ERROR;
		SIPDEBUG("SIP DECODE MESSAGE No Text in message. Returning fail\n");
		return SipFail;
	}

	/* Find end of response/request line */
	linestartindex = i;
	j=linestartindex;
	while(j<myinputindex-1)
	{
		if((message[j]=='\n')||(message[j]=='\r'))
			if(!(isspace((int) message[j+1]))) break;
		j++;
	}
	if(j==myinputindex-1)
	{
		if((opt->dOption & SIP_OPT_PARTIAL) != SIP_OPT_PARTIAL)
		{
			/* Message with a single line */
			SIPDEBUG("SIP DECODE MESSAGE Single line message." \
					" Returning fail.\n");
#ifndef SIP_INCOMPLETE_MSG_CHECK
			*err = E_PARSER_ERROR;
#else
			*err = E_INCOMPLETE;
#endif
			sip_listDeleteAll(dParserParams.pGCList,&dErr);
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,\
					&dErr);
			return SipFail;
		}
		else
		{
			/* For partial pasing, rewind the header begin pointer
			   till first CRLF */
			while(isspace((int)message[j-1])) j--;
			singleLineMessage = 1;
		}
	}
	headerstartindex = j+1;
	/* Length of the message : myinputindex-1
	   Index of request line : linestartindex
	   Starting of headers   : headerstartindex */

	/* Now find if line is request line or a statusline */
	/* If message starts with "SIP" it is likely to be a response message
	   else assume it is a request          */
	if((((message[linestartindex]=='S')||\
					(message[linestartindex]=='s')))\
			&&((message[linestartindex+1]=='I')||\
				(message[linestartindex+1]=='i'))\
			&&((message[linestartindex+2]=='P')||\
				(message[linestartindex+2]=='p')))
	{
		dParserParams.pSipMessage=(SipMessage *)fast_memget(DECODE_MEM_ID,\
				sizeof(SipMessage),err);
		if((dParserParams.pSipMessage) == SIP_NULL)
		{
			*err=E_NO_MEM;
			return SipFail;
		}
		(dParserParams.pSipMessage)->dType= SipMessageResponse;
	}
	else
	{
		dParserParams.pSipMessage=(SipMessage *)fast_memget(DECODE_MEM_ID,\
				sizeof(SipMessage),err);
		if((dParserParams.pSipMessage) == SIP_NULL)
		{
			*err=E_NO_MEM;
			return SipFail;
		}
		(dParserParams.pSipMessage)->dType = SipMessageRequest;
	}
	(dParserParams.pError)=(SipError*)fast_memget(DECODE_MEM_ID, \
												  sizeof(SipError),err);
	if(dParserParams.pError==SIP_NULL)
	{
		*err=E_NO_MEM;
		return SipFail;
	}
	(*(dParserParams.pError))=E_NO_ERROR;
	HSS_INITREF(dParserParams.pSipMessage->dRefCount);
	/* Point parser input Buffer to Start of Request/status line */
	if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == SIP_OPT_DBLNULLBUFFER)
	{
		pReqLineParserBuffer = &message[linestartindex];
		reqSavedChars[0] = \
			pReqLineParserBuffer[headerstartindex-linestartindex];
		reqSavedChars[1] = \
			pReqLineParserBuffer[headerstartindex-linestartindex+1];
		pReqLineParserBuffer[headerstartindex-linestartindex] = '\0';
		pReqLineParserBuffer[headerstartindex-linestartindex+1] = '\0';
	}
	else
	{
		pReqLineParserBuffer = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
				headerstartindex-linestartindex+2, err);
		if(pReqLineParserBuffer == SIP_NULL)
			return SipFail;
		strncpy(pReqLineParserBuffer,&message[linestartindex],\
				headerstartindex-linestartindex);
		pReqLineParserBuffer[headerstartindex-linestartindex] = '\0';
		pReqLineParserBuffer[headerstartindex-linestartindex+1] = '\0';
	}


	if((dParserParams.pSipMessage)->dType == SipMessageResponse)
	{
		/* Initialize message structure for status line parser */
		(dParserParams.pSipMessage)->u.pResponse = (SipRespMessage*)\
			fast_memget(DECODE_MEM_ID,sizeof(SipRespMessage),err);
		if((dParserParams.pSipMessage)->u.pResponse == SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF\
			((dParserParams.pSipMessage)->u.pResponse->dRefCount);
		(dParserParams.pSipMessage)->u.pResponse->pStatusLine \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr = (SipGeneralHeader*) \
			fast_memget(DECODE_MEM_ID,sizeof(SipGeneralHeader),err);
		if((dParserParams.pSipMessage)->pGeneralHdr == SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF\
			((dParserParams.pSipMessage)->pGeneralHdr->dRefCount);
		(dParserParams.pSipMessage)->u.pResponse->pResponseHdr = (\
										 SipRespHeader*)fast_memget(\
										 DECODE_MEM_ID,sizeof(SipRespHeader),\
										 err);
		if((dParserParams.pSipMessage)->u.pResponse->pResponseHdr \
				== SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF((dParserParams.pSipMessage)->\
				u.pResponse->pResponseHdr->dRefCount);
		/* Message might be a response
		   Set message Type to request. If status line parser succeeds,
		   it will set message Type to response*/
		(dParserParams.pSipMessage)->dType = SipMessageRequest;
		*(dParserParams.pError)=E_NO_ERROR;
		if (sip_lex_Statusline_scan_buffer(pReqLineParserBuffer,\
					headerstartindex-linestartindex+2) != 0)
		{
			*(dParserParams.pError)=E_NO_MEM;
			return SipFail;
		}
		pSipParserInput = pReqLineParserBuffer;
		glbSipParserStatuslineparse((void *)&dParserParams);
		sip_lex_Statusline_release_buffer();
		if( (opt->dOption & SIP_OPT_DBLNULLBUFFER) ==\
				SIP_OPT_DBLNULLBUFFER)
		{
			pReqLineParserBuffer[headerstartindex-linestartindex] = \
				reqSavedChars[0];
			pReqLineParserBuffer[headerstartindex-linestartindex+1] = \
				reqSavedChars[1];
		}
		else
		{
			fast_memfree(DECODE_MEM_ID, pReqLineParserBuffer, SIP_NULL);
		}
		/* free all resources collected by GC in parsing phase */
		if(sip_listDeleteAll(dParserParams.pGCList, err)==SipFail)
		{
			return SipFail;
		}

		if(*(dParserParams.pError)!=E_NO_ERROR)
		{
			*err=*(dParserParams.pError);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					pGeneralHdr, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pResponse-> pResponseHdr, SIP_NULL);
			sip_freeSipStatusLine\
				((dParserParams.pSipMessage)->u.pResponse->pStatusLine);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pResponse, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage),\
					SIP_NULL);
			fast_memfree(DECODE_MEM_ID,\
					(SIP_Pvoid *)dParserParams.pGCList,&dErr);
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
					dParserParams.pError,&dErr);
			return SipFail;
		}
		if((dParserParams.pSipMessage)->dType != SipMessageResponse)
		{
			SIPDEBUG("SIP DECODE MESSAGE Invalid status line." \
					" Returning fail.\n");
			fast_memfree(DECODE_MEM_ID,\
					(dParserParams.pSipMessage)->pGeneralHdr, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,\
					(dParserParams.pSipMessage)->u.pResponse->\
					pResponseHdr, SIP_NULL);
			sip_freeSipStatusLine((dParserParams.pSipMessage)-> \
					u.pResponse-> pStatusLine);
			fast_memfree(DECODE_MEM_ID,\
					(dParserParams.pSipMessage)->u.pResponse, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,\
					(dParserParams.pSipMessage),SIP_NULL);
			*err = E_PARSER_ERROR;
			fast_memfree(DECODE_MEM_ID,\
					(SIP_Pvoid *)dParserParams.pGCList,&dErr);
			fast_memfree(DECODE_MEM_ID,\
					(SIP_Pvoid *)dParserParams.pError,&dErr);
			return SipFail;
		}
		/* Prepare structure to read in response. */
		if(sip_listInit(&(dParserParams.pSipMessage)->slOrderInfo,\
					__sip_freeSipHeaderOrderInfo,err)==SipFail)
			return SipFail;
		/* Initialize general headers and lists */
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptHdr,__sip_freeSipAcceptHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptEncoding,__sip_freeSipAcceptEncodingHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptLang,__sip_freeSipAcceptLangHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContactHdr,__sip_freeSipContactHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentDispositionHdr,\
					__sip_freeSipContentDispositionHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slReasonHdr,__sip_freeSipReasonHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slRecordRouteHdr,__sip_freeSipRecordRouteHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slViaHdr,__sip_freeSipViaHeader,err)==SipFail)
			return SipFail;
#ifdef SIP_PRIVACY
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPAssertIdHdr,__sip_freeSipPAssertIdHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPPreferredIdHdr,__sip_freeSipPPreferredIdHeader,err)\
				==SipFail)
			return SipFail;
#endif /* #ifdef SIP_PRIVACY */

		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentEncodingHdr,__sip_freeSipContentEncodingHeader\
					,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slSupportedHdr,__sip_freeSipSupportedHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentLanguageHdr,\
					__sip_freeSipContentLanguageHeader,err)== SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slCallInfoHdr,__sip_freeSipCallInfoHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAllowHdr,__sip_freeSipAllowHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slRequireHdr,__sip_freeSipRequireHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slUnknownHdr,__sip_freeSipUnknownHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slBadHdr,__sip_freeSipBadHeader,err)==SipFail)
			return SipFail;
#ifdef SIP_3GPP
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slServiceRouteHdr,__sip_freeSipServiceRouteHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPathHdr,__sip_freeSipPathHeader,err)\
				==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->pGeneralHdr->pPcVectorHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pPanInfoHdr= SIP_NULL;
#endif
#ifdef SIP_IMPP
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAllowEventsHdr,__sip_impp_freeSipAllowEventsHeader,\
					err)==SipFail)
			return SipFail;
#endif
#ifdef SIP_DCS
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsAnonymityHdr,__sip_dcs_freeSipDcsAnonymityHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsRemotePartyIdHdr,\
					__sip_dcs_freeSipDcsRemotePartyIdHeader, err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsRpidPrivacyHdr,\
					__sip_dcs_freeSipDcsRpidPrivacyHeader, err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsStateHdr,__sip_dcs_freeSipDcsStateHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsMediaAuthorizationHdr,\
					__sip_dcs_freeSipDcsMediaAuthorizationHeader,\
					err)==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsLaesHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsGateHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsBillingIdHdr =\
			SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsBillingInfoHdr=\
			SIP_NULL;
#endif
		(dParserParams.pSipMessage)->pGeneralHdr->pCallidHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pReplyToHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pCseqHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDateHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pEncryptionHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pExpiresHdr \
			= SIP_NULL;
#ifdef SIP_SESSIONTIMER
		(dParserParams.pSipMessage)->pGeneralHdr->pMinSEHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pSessionExpiresHdr \
			= SIP_NULL;
#endif

		(dParserParams.pSipMessage)->pGeneralHdr->pFromHeader \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pTimeStampHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pToHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pContentLengthHdr\
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pContentTypeHdr\
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pMimeVersionHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pOrganizationHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pUserAgentHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pRetryAfterHdr \
			= SIP_NULL;
#ifdef SIP_PRIVACY
		(dParserParams.pSipMessage)->pGeneralHdr->pPrivacyHdr = SIP_NULL;
#endif
#ifdef SIP_3GPP
		(dParserParams.pSipMessage)->pGeneralHdr->pPcfAddrHdr = SIP_NULL;
#endif
		/* Initialize response headers and lists */
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slProxyAuthenticateHdr, \
					__sip_freeSipProxyAuthenticateHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse->  \
					pResponseHdr->slUnsupportedHdr,\
					__sip_freeSipUnsupportedHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr-> slWarningHeader,\
					__sip_freeSipWarningHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slWwwAuthenticateHdr, \
					__sip_freeSipWwwAuthenticateHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slAuthenticationInfoHdr, \
					__sip_freeSipAuthenticationInfoHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr-> slErrorInfoHdr,\
					__sip_freeSipErrorInfoHeader,err)\
				==SipFail)
			return SipFail;
#ifdef SIP_DCS
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr-> slSessionHdr,\
					__sip_dcs_freeSipSessionHeader,err)==SipFail)
			return SipFail;
#endif

		(dParserParams.pSipMessage)->u.pResponse->pResponseHdr->\
			pServerHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pResponse->pResponseHdr->\
			pMinExpiresHdr = SIP_NULL;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr-> slAuthorizationHdr,\
					__sip_freeSipAuthorizationHeader,err)==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->u.pResponse->pResponseHdr->\
			pRSeqHdr = SIP_NULL;
		if(sip_listInit(&(dParserParams.pSipMessage)->slMessageBody,\
					__sip_freeSipMsgBody,err)==SipFail)
			return SipFail;
		
#ifdef SIP_3GPP
        if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slPAssociatedUriHdr,\
					__sip_freeSipPAssociatedUriHeader,err)==SipFail)
			return SipFail;
#endif
#ifdef SIP_CONGEST
        if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slAcceptRsrcPriorityHdr,\
					__sip_freeSipAcceptRsrcPriorityHeader,err)==SipFail)
			return SipFail;
#endif
#ifdef SIP_SECURITY
	if(sip_listInit(&(dParserParams.pSipMessage)->u.pResponse-> \
					pResponseHdr->slSecurityServerHdr,\
					__sip_freeSipSecurityServerHeader,err)==SipFail)
			return SipFail;
#endif
	}
	else
	{
		/* Initailize message structure for request line parsing */
		(dParserParams.pSipMessage)->u.pRequest = (SipReqMessage*)\
			fast_memget(DECODE_MEM_ID,sizeof(SipReqMessage),err);
		if((dParserParams.pSipMessage)->u.pRequest == SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF((dParserParams.pSipMessage)->u.pRequest->dRefCount);
		(dParserParams.pSipMessage)->u.pRequest->pRequestLine =\
			SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr = (SipGeneralHeader*) \
			fast_memget(DECODE_MEM_ID,sizeof(SipGeneralHeader),err);
		if((dParserParams.pSipMessage)->pGeneralHdr == SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF\
			((dParserParams.pSipMessage)->pGeneralHdr->dRefCount);
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr = \
			(SipReqHeader*) \
			fast_memget(DECODE_MEM_ID,sizeof(SipReqHeader),err);
		if((dParserParams.pSipMessage)->u.pRequest->pRequestHdr\
				== SIP_NULL)
		{
			return SipFail;
		}
		HSS_INITREF((dParserParams.pSipMessage)\
				->u.pRequest->pRequestHdr->dRefCount);
		/* parse request line */
		/* Message likely be a request. Set Type to response
		   and check if request line parser sets it to request */
		(dParserParams.pSipMessage)->dType = SipMessageResponse;
		*(dParserParams.pError) = E_NO_ERROR;
		if (sip_lex_Reqline_scan_buffer(pReqLineParserBuffer, \
					headerstartindex-linestartindex+2) != 0)
		{
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					pGeneralHdr, SIP_NULL);
			sip_freeSipReqLine((dParserParams.pSipMessage)->\
					u.pRequest->pRequestLine);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest->pRequestHdr, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest,SIP_NULL);
			fast_memfree\
				(DECODE_MEM_ID,(dParserParams.pSipMessage),SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
					dParserParams.pGCList, &dErr);
			*(dParserParams.pError) = E_NO_MEM;
			return SipFail;
		}
		pSipParserInput = pReqLineParserBuffer;

		sip_lex_Reqline_reset_state();
		glbSipParserReqlineparse((void *)&dParserParams);

		sip_lex_Reqline_release_buffer();
		if((opt->dOption & SIP_OPT_DBLNULLBUFFER) ==\
				SIP_OPT_DBLNULLBUFFER)
		{
			pReqLineParserBuffer[headerstartindex-linestartindex] = \
				reqSavedChars[0];
			pReqLineParserBuffer[headerstartindex-linestartindex+1] = \
				reqSavedChars[1];
		}
		else
		{
			fast_memfree(DECODE_MEM_ID,pReqLineParserBuffer,SIP_NULL);
		}
		/* free all resources collected by GC in parsing phase */
		if(sip_listDeleteAll(dParserParams.pGCList, err)==SipFail)
			return SipFail;
		if(*(dParserParams.pError) != E_NO_ERROR)
		{
			*err = *(dParserParams.pError);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					pGeneralHdr, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest->pRequestHdr, SIP_NULL);
			sip_freeSipReqLine((dParserParams.pSipMessage)->\
					u.pRequest->pRequestLine);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest,SIP_NULL);
			fast_memfree\
				(DECODE_MEM_ID,(dParserParams.pSipMessage),SIP_NULL);
			fast_memfree\
				(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,\
				 &dErr);
			fast_memfree\
				(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pError,&dErr);
			return SipFail;
		}
		if((dParserParams.pSipMessage)->dType != SipMessageRequest)
		{
			/* Not a valid sip message */
			SIPDEBUG("SIP DECODE MESSAGE Invalid request line."\
					" Returning fail.\n");
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					pGeneralHdr, SIP_NULL);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest->pRequestHdr, SIP_NULL);
			sip_freeSipReqLine\
				((dParserParams.pSipMessage)->u.pRequest->pRequestLine);
			fast_memfree(DECODE_MEM_ID,(dParserParams.pSipMessage)-> \
					u.pRequest,SIP_NULL);
			fast_memfree\
				(DECODE_MEM_ID,(dParserParams.pSipMessage),SIP_NULL);
			(dParserParams.pSipMessage)=SIP_NULL;
			*err = E_PARSER_ERROR;
			fast_memfree(DECODE_MEM_ID,\
					(SIP_Pvoid *)dParserParams.pGCList, &dErr);
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
					dParserParams.pError,&dErr);
			return SipFail;
		}
		/* Initialize all request pHeader structures and lists */
		if(sip_listInit(&(dParserParams.pSipMessage)->slOrderInfo,\
					__sip_freeSipHeaderOrderInfo,err)==SipFail)
			return SipFail;
		/* General header initialization */
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slUnknownHdr,__sip_freeSipUnknownHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slBadHdr,__sip_freeSipBadHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptEncoding,__sip_freeSipAcceptEncodingHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptHdr,__sip_freeSipAcceptHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentDispositionHdr,\
					__sip_freeSipContentDispositionHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slReasonHdr,__sip_freeSipReasonHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptEncoding,__sip_freeSipAcceptEncodingHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAcceptLang,__sip_freeSipAcceptLangHeader,err)==\
				SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContactHdr,__sip_freeSipContactHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slRecordRouteHdr,__sip_freeSipRecordRouteHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slViaHdr,__sip_freeSipViaHeader,err)==SipFail)
			return SipFail;
#ifdef SIP_PRIVACY
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPAssertIdHdr,__sip_freeSipPAssertIdHeader,err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPPreferredIdHdr,__sip_freeSipPPreferredIdHeader,err)\
				==SipFail)
			return SipFail;
#endif /* # ifdef SIP_PRIVACY */

		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentEncodingHdr,\
					__sip_freeSipContentEncodingHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slSupportedHdr,__sip_freeSipSupportedHeader,err)==\
				SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slContentLanguageHdr,__sip_freeSipContentLanguageHeader\
					,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slCallInfoHdr,__sip_freeSipCallInfoHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slRequireHdr,__sip_freeSipRequireHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAllowHdr,__sip_freeSipAllowHeader,err)==SipFail)
			return SipFail;
#ifdef SIP_3GPP						
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slServiceRouteHdr,__sip_freeSipServiceRouteHeader,err)
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slPathHdr,__sip_freeSipPathHeader,err)==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->pGeneralHdr->pPcVectorHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pPanInfoHdr= SIP_NULL;
#endif						
#ifdef SIP_IMPP
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slAllowEventsHdr,__sip_impp_freeSipAllowEventsHeader,\
					err)==SipFail)
			return SipFail;
#endif
#ifdef SIP_DCS
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsAnonymityHdr,__sip_dcs_freeSipDcsAnonymityHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsRemotePartyIdHdr,\
					__sip_dcs_freeSipDcsRemotePartyIdHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsRpidPrivacyHdr,\
					__sip_dcs_freeSipDcsRpidPrivacyHeader, err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsStateHdr,__sip_dcs_freeSipDcsStateHeader,\
					err)==SipFail)
			return SipFail;
#endif
		(dParserParams.pSipMessage)->pGeneralHdr->pCallidHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pReplyToHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pCseqHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDateHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pEncryptionHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pExpiresHdr = \
			SIP_NULL;
#ifdef SIP_SESSIONTIMER
		(dParserParams.pSipMessage)->pGeneralHdr->pMinSEHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pSessionExpiresHdr\
			= SIP_NULL;
#endif

		(dParserParams.pSipMessage)->pGeneralHdr->pFromHeader = \
			SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pTimeStampHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pToHdr = SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pContentLengthHdr\
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pContentTypeHdr\
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pMimeVersionHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pOrganizationHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pUserAgentHdr \
			= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pRetryAfterHdr \
			= SIP_NULL;
#ifdef SIP_PRIVACY
		(dParserParams.pSipMessage)->pGeneralHdr->pPrivacyHdr = SIP_NULL;
#endif
#ifdef SIP_3GPP
		(dParserParams.pSipMessage)->pGeneralHdr->pPcfAddrHdr = SIP_NULL;
#endif
		/* Request Header initialization */
		(dParserParams.pSipMessage)\
			->u.pRequest->pRequestHdr->pReplacesHdr = SIP_NULL;
#ifdef SIP_IMPP
		(dParserParams.pSipMessage)\
			->u.pRequest->pRequestHdr->pEventHdr = SIP_NULL;
#endif
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slProxyRequireHdr,\
					__sip_freeSipProxyRequireHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->\
					u.pRequest->pRequestHdr->\
					slRouteHdr,__sip_freeSipRouteHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest\
					->pRequestHdr->slWwwAuthenticateHdr,\
					__sip_freeSipWwwAuthenticateHeader,err)==SipFail)
			return SipFail;
#ifdef SIP_CCP
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slAcceptContactHdr,\
					__sip_ccp_freeSipAcceptContactHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slRejectContactHdr,\
					__sip_ccp_freeSipRejectContactHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slRequestDispositionHdr,\
					__sip_ccp_freeSipRequestDispositionHeader,err)==SipFail)
			return SipFail;
#endif
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slAlertInfoHdr,__sip_freeSipAlertInfoHeader,\
					err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->\
					u.pRequest->pRequestHdr->\
					slInReplyToHdr,__sip_freeSipInReplyToHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slAlsoHdr,__sip_freeSipAlsoHeader, err)\
				==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest\
					->pRequestHdr->slProxyAuthorizationHdr,\
					__sip_freeSipProxyAuthorizationHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slAuthorizationHdr,\
					__sip_freeSipAuthorizationHeader,err)==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pReferToHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->\
			pRequestHdr->pReferredByHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->pHideHdr\
			= SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pMaxForwardsHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pPriorityHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pRespKeyHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pSubjectHdr = SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->pRackHdr\
			= SIP_NULL;
#ifdef SIP_DCS
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pDcsOspsHdr= SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr-> \
			pDcsTracePartyIdHdr= SIP_NULL;
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pDcsRedirectHdr= SIP_NULL;
		if(sip_listInit(&(dParserParams.pSipMessage)->pGeneralHdr->\
					slDcsMediaAuthorizationHdr,\
					__sip_dcs_freeSipDcsMediaAuthorizationHeader,\
					err)==SipFail)
			return SipFail;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsLaesHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsGateHdr= SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsBillingIdHdr=\
			SIP_NULL;
		(dParserParams.pSipMessage)->pGeneralHdr->pDcsBillingInfoHdr\
			= SIP_NULL;
#endif
#ifdef SIP_IMPP
		(dParserParams.pSipMessage)->u.pRequest->pRequestHdr->\
			pSubscriptionStateHdr = SIP_NULL;
#endif
#ifdef SIP_CONGEST
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slRsrcPriorityHdr,\
					__sip_freeSipRsrcPriorityHeader,err)==SipFail)
			return SipFail;
#endif

#ifdef SIP_CONF
		/* Join header */
        (dParserParams.pSipMessage)\
			->u.pRequest->pRequestHdr->pJoinHdr = SIP_NULL;
#endif
#ifdef SIP_SECURITY
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
                			pRequestHdr->slSecurityClientHdr,\
					__sip_freeSipSecurityClientHeader,err)==SipFail)
			return SipFail;
		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slSecurityVerifyHdr,\
					__sip_freeSipSecurityVerifyHeader,err)==SipFail)
			return SipFail;
#endif

#ifdef SIP_3GPP
        (dParserParams.pSipMessage)\
			->u.pRequest->pRequestHdr->pPCalledPartyIdHdr = SIP_NULL;

		if(sip_listInit(&(dParserParams.pSipMessage)->u.pRequest->\
					pRequestHdr->slPVisitedNetworkIdHdr,\
					__sip_freeSipPVisitedNetworkIdHeader,err)==SipFail)
			return SipFail;
#endif

		if(sip_listInit(&(dParserParams.pSipMessage)->slMessageBody,\
					__sip_freeSipMsgBody,err)==SipFail)
			return SipFail;
	}

	/* Message initializations done */
	/* If partial parsing is enabled and singleLineMessage is true
	   return success here */
	if(singleLineMessage == 1)
	{
		*(ppSipMessage)=dParserParams.pSipMessage;
		fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
				dParserParams.pGCList,&dErr);
		fast_memfree(DECODE_MEM_ID,dParserParams.pError,&dErr);
		return SipSuccess;
	}

	/* Initialize number of incorrect headers to zero */
	(dParserParams.pSipMessage)->dIncorrectHdrsCount = 0;
	(dParserParams.pSipMessage)->dEntityErrorCount = 0;

	*(dParserParams.pError) = E_NO_ERROR;
	dParseEachHeaderParam.pParserStruct = &dParserParams;
	dParseEachHeaderParam.pList = &glbSipParserHdrTypeList;
	if ((opt->dOption & SIP_OPT_SELECTIVE)==SIP_OPT_SELECTIVE)
		if(pContext!=SIP_NULL)
			if(pContext->pList!=SIP_NULL)
				dParseEachHeaderParam.pList = pContext->pList;
	if ((opt->dOption & SIP_OPT_BADMESSAGE)==SIP_OPT_BADMESSAGE)
		dParseEachHeaderParam.ignoreErrors = 1;
	else
		dParseEachHeaderParam.ignoreErrors = 0;
	dParseEachHeaderParam.noSelectiveCallBacks = SipFail;

	bodystartindex=headerstartindex;
	headerendindex = headerstartindex;
	lineEndIndex = headerstartindex;
	messageBodyFlag = 0;
	headerParserBuffer = &message[headerstartindex];
	/*
	 * Initilize the pParserbuffer=message[headerstartIndex]
	 * if the DBUF flag is set; else use a copy.
	 * Continue until u reach message body start
	 */
	while ((messageBodyFlag==0) && (dRetFromGetHdrLine=getHeaderLineIndex(\
					&message[headerstartindex], messageLength,\
					&headerendindex, &lineEndIndex, &bodystartindex, &dHeader,\
					&messageBodyFlag, err)) == SipSuccess )
	{
		if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == \
				SIP_OPT_DBLNULLBUFFER)
		{
			headerSavedChars[0]=message[lineEndIndex];
			headerSavedChars[1]=message[lineEndIndex+1];
			message[lineEndIndex] = '\0';
			message[lineEndIndex+1] = '\0';
			headerParserBuffer = &message[headerstartindex];
			scanLength=lineEndIndex - headerstartindex;
		}

		glbSipParserParseEachHeader(&dHeader, headerParserBuffer,\
				scanLength,opt, &dParseEachHeaderParam);

		/* free unknown header elements */
		if (dHeader.pName !=SIP_NULL)
		{
			fast_memfree(DECODE_MEM_ID,dHeader.pName,SIP_NULL);
			dHeader.pName=SIP_NULL;
		}

		if (dHeader.pBody !=SIP_NULL)
		{
			fast_memfree(DECODE_MEM_ID,dHeader.pBody,SIP_NULL);
			dHeader.pBody=SIP_NULL;
		}

		if(*(dParserParams.pError) != E_NO_ERROR)
		{
			*err = *(dParserParams.pError);
			SIPDEBUG("SIP DECODE MESSAGE Error in parsing headers.\n");
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
					dParserParams.pGCList,&dErr);
			fast_memfree(DECODE_MEM_ID,dParserParams.pError,&dErr);
			sip_freeSipMessage(dParserParams.pSipMessage);
			return SipFail;
		}

		if((opt->dOption & SIP_OPT_DBLNULLBUFFER) == \
				SIP_OPT_DBLNULLBUFFER)
		{
			message[lineEndIndex]=headerSavedChars[0];
			message[lineEndIndex+1]=headerSavedChars[1];
		}

		headerstartindex=headerendindex;

		if (headerendindex >= messageLength)
			break;
	}

	/*Check for a possible error from the getHeaderLineAtIndex call*/
	if (dRetFromGetHdrLine == SipFail)
	{
		/*We need to free the Unknown Hdr components
		  namely the name and body*/
		/* free unknown header elements */
		if (dHeader.pName !=SIP_NULL)
		{
			fast_memfree(DECODE_MEM_ID,dHeader.pName,SIP_NULL);
			dHeader.pName=SIP_NULL;
		}

		if (dHeader.pBody !=SIP_NULL)
		{
			fast_memfree(DECODE_MEM_ID,dHeader.pBody,SIP_NULL);
			dHeader.pBody=SIP_NULL;
		}
	}

	if(*err != E_NO_ERROR)
	{
		SIPDEBUG("SIP DECODE MESSAGE Error in splitting headers.\n");
		fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
				dParserParams.pGCList,&dErr);
		fast_memfree(DECODE_MEM_ID,dParserParams.pError,&dErr);
		if(*err == E_INCOMPLETE)
		{
			*(ppSipMessage)=dParserParams.pSipMessage;
		}
		else
		{
			sip_freeSipMessage(dParserParams.pSipMessage);
		}
		return SipFail;
	}
#ifdef SIP_INCOMPLETE_MSG_CHECK
	if(messageBodyFlag == 0)
	{
		SipGeneralHeader *g;
		g = (dParserParams.pSipMessage)->pGeneralHdr;
		/* check if message has content length header or not */
		if(g->pContentLengthHdr != SIP_NULL)
		{
			/* content length header is present. so check the 
			 * length value */
			if(g->pContentLengthHdr->dLength > 0)
			{
				*err = E_INCOMPLETE;
				if (pContext != SIP_NULL)
					pContext->dRemainingLength = g->pContentLengthHdr->dLength;
				*(ppSipMessage)=dParserParams.pSipMessage;
				fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,\
						&dErr);
				fast_memfree(DECODE_MEM_ID,dParserParams.pError,&dErr);
				return SipFail;
			}
		}
	}
#endif


	if(messageBodyFlag==1)
	{
		SIP_U32bit dType=0;
		SipGeneralHeader *g;
		SIP_S8bit *messagebody = SIP_NULL;

		if ((dParserParams.pSipMessage)->dType==SipMessageResponse)
			dType = 1;
		g = (dParserParams.pSipMessage)->pGeneralHdr;
		/* see if message has a content-Length */
		if (g->pContentLengthHdr!=SIP_NULL)
			clen=g->pContentLengthHdr->dLength;
		/*If no content-Length specified, take till end of packet */
		else
			clen=myinputindex-bodystartindex;
		if(clen>(myinputindex-bodystartindex))
		{
			/*content Length less than that claimed in the Header */
			/* take till end of packet */

#ifdef SIP_INCOMPLETE_MSG_CHECK
			/*
			 * Here we attempt to be strict with Content Len checking
			 * if content Length header is less than body, maybe
			 * body is incomplete, so return E_MAYBE_INCOMPLETE
			 */

			*err = E_INCOMPLETE;
			if (pContext != SIP_NULL)
				pContext->dRemainingLength = clen - \
					(myinputindex-bodystartindex);
			*(ppSipMessage)=dParserParams.pSipMessage;
			fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,\
					&dErr);
			fast_memfree(DECODE_MEM_ID,dParserParams.pError,&dErr);
			return SipFail;
#else
			clen=myinputindex-bodystartindex;
#endif
		}
		contentendindex = bodystartindex+clen;
		/* If anything remains, copy it to next message */
		if(contentendindex<myinputindex)
		{
			*nextmesg = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
					myinputindex-(contentendindex)+1,err);
			if(*nextmesg == SIP_NULL)
				return SipFail;
			strncpy(*nextmesg,&message[contentendindex],\
					myinputindex-(contentendindex));
			(*nextmesg)[myinputindex-(contentendindex)] = '\0';
			if(pContext != SIP_NULL)
				pContext->dNextMessageLength = (myinputindex-contentendindex);
		}

		/*
		 * Start and end of message Body determined.
		 * Next message created
		 */

		/* Check if encryption Header exists. */
		if(g->pEncryptionHdr!=SIP_NULL)
		{
			/* Message is encrypted */
			SIP_S8bit *encinbuffer;
			SIP_U32bit outlen;

			/*
			 * Copy Body to a Buffer and pass to users's
			 * decrypt function
			 */
			encinbuffer = &message[bodystartindex];
			if(sip_decryptBuffer((dParserParams.pSipMessage), \
						encinbuffer, clen,&encoutbuffer, &outlen) == \
					SipSuccess)
			{
				/* Decrypted Buffer in encoutbuffer */
				/* check for presence of headers */
				if(outlen > 2)
				{
					if((encoutbuffer[0]=='\r')&&(encoutbuffer[1]==\
								'\n'))
					{
						/* No headers. Only Body present */
						messagebody = encoutbuffer;
						bodystartindex = 2;
						clen = outlen - 2;
					}
					else
					{
						/* Set parser Buffer to point to Start of headers */
						SIP_S8bit *pEncoutParserBuffer;
						pEncoutParserBuffer = (SIP_S8bit *) \
							fast_memget(DECODE_MEM_ID,outlen+2,err);
						if(pEncoutParserBuffer == SIP_NULL)
						{
							sip_freeString(encoutbuffer);
							return SipFail;
						}
						strncpy(pEncoutParserBuffer, encoutbuffer, \
								outlen);
						pEncoutParserBuffer[outlen] = '\0';
						pEncoutParserBuffer[outlen+1] = '\0';
						/*
						 * NEW parser code starts here
						 */

						*(dParserParams.pError) = E_NO_ERROR;
						dParseEachHeaderParam.pParserStruct = \
							&dParserParams;
						dParseEachHeaderParam.pList = \
							&glbSipParserHdrTypeList;
						if ((opt->dOption & SIP_OPT_SELECTIVE)==\
								SIP_OPT_SELECTIVE)
							if(pContext!=SIP_NULL)
								if(pContext->pList!=SIP_NULL)
									dParseEachHeaderParam.pList = \
										pContext->pList;
						if ((opt->dOption & SIP_OPT_BADMESSAGE)== \
								SIP_OPT_BADMESSAGE)
							dParseEachHeaderParam.ignoreErrors = 1;
						else
							dParseEachHeaderParam.ignoreErrors = 0;

						dParseEachHeaderParam.noSelectiveCallBacks=\
							SipFail;

						bodystartindex = 0;
						headerstartindex = 0;
						headerendindex = headerstartindex;
						lineEndIndex = headerstartindex;
						messageBodyFlag = 0;
						headerParserBuffer = \
							&pEncoutParserBuffer[headerstartindex];

						while ((messageBodyFlag==0) && \
								getHeaderLineIndex(\
									&pEncoutParserBuffer[headerstartindex],\
									outlen, &headerendindex, &lineEndIndex,\
									&bodystartindex, &dHeader, \
									&messageBodyFlag, dParserParams.pError)\
								== SipSuccess )
						{
							if((opt->dOption & \
										SIP_OPT_DBLNULLBUFFER) == \
									SIP_OPT_DBLNULLBUFFER)
							{
								headerSavedChars[0] = \
									pEncoutParserBuffer[lineEndIndex];
								headerSavedChars[1] = \
									pEncoutParserBuffer[lineEndIndex+1];
								pEncoutParserBuffer[lineEndIndex]=\
									'\0';
								pEncoutParserBuffer[lineEndIndex+1] = '\0';
								headerParserBuffer = \
									&pEncoutParserBuffer[headerstartindex];
								scanLength=lineEndIndex - headerstartindex;
							}

							/*
							 * remove existing header
							 */
							glbSipParserRemoveExistingHeaders( \
									&dHeader, &dParserParams);

							glbSipParserParseEachHeader(&dHeader, \
									headerParserBuffer, scanLength, opt, \
									&dParseEachHeaderParam);

							if((opt->dOption & \
										SIP_OPT_DBLNULLBUFFER) == \
									SIP_OPT_DBLNULLBUFFER)
							{
								pEncoutParserBuffer[lineEndIndex]=\
									headerSavedChars[0];
								pEncoutParserBuffer[lineEndIndex+1] =\
									headerSavedChars[1];
							}

							headerstartindex=headerendindex;

							/* free unknown header elements */
							if (dHeader.pName !=SIP_NULL)
								fast_memfree(DECODE_MEM_ID, \
										dHeader.pName,SIP_NULL);
							if (dHeader.pBody !=SIP_NULL)
								fast_memfree(DECODE_MEM_ID, \
										dHeader.pBody,SIP_NULL);
							if(*(dParserParams.pError)!=E_NO_ERROR)
							{
								*err = *(dParserParams.pError);
								SIPDEBUG(\
										"SIP DECODE MESSAGE Error in \
										parsing headers.\n");
								fast_memfree(DECODE_MEM_ID,\
										(SIP_Pvoid *)dParserParams.pGCList,\
										&dErr);
								fast_memfree(DECODE_MEM_ID,\
										dParserParams.pError,&dErr);
								sip_freeSipMessage(\
										dParserParams.pSipMessage);
								return SipFail;
							}
							if (headerendindex > outlen)
								break;
						}
						if(*(dParserParams.pError) != E_NO_ERROR)
						{
							*err = *(dParserParams.pError);
							SIPDEBUG("SIP DECODE MESSAGE Error"\
									" in parsing headers.\n");
							fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
									dParserParams.pGCList,&dErr);
							fast_memfree\
								(DECODE_MEM_ID,dParserParams.pError,&dErr);
							sip_freeSipMessage(dParserParams.pSipMessage);
							return SipFail;
						}
						/* NEW parser code ends here */

						fast_memfree(DECODE_MEM_ID, \
								pEncoutParserBuffer, SIP_NULL);

						messagebody = encoutbuffer;
						clen = outlen-bodystartindex;
					}
				}
			}
			else
			{
				/* Could not decrypt, stuff encrypted portion to
				 * unknown pBody
				 */
				SipMsgBody *msgbody;
				SipUnknownMessage *unmsg;

				encoutbuffer = SIP_NULL;
				if(sip_initSipMsgBody(&msgbody,SipUnknownBody,err) \
						==SipFail)
					return SipFail;
				if(sip_initSipUnknownMessage(&unmsg, err) \
						== SipFail)
					return SipFail;
				unmsg->pBuffer = (SIP_S8bit *) \
					fast_memget(DECODE_MEM_ID,clen+1,err);
				if(unmsg->pBuffer==SIP_NULL)
				{
					*(dParserParams.pError) = E_NO_MEM;
					return SipFail;
				}
				memcpy(unmsg->pBuffer, &message[bodystartindex], clen);
				unmsg->pBuffer[clen]='\0';
				unmsg->dLength = clen;
				msgbody->u.pUnknownMessage = unmsg;
				if(sip_listAppend(&(dParserParams.pSipMessage)->\
							slMessageBody, (SIP_Pvoid)msgbody,err)==SipFail)
					return SipFail;
				messageBodyFlag = 0;
			}
		} /* if encrypted message */
		else
		{
			/* normal message */
			messagebody = message;
		}

		if(clen == 0)
			messageBodyFlag = 0;
		if(messageBodyFlag == 1)
		{
			if((opt->dOption & SIP_OPT_NOPARSEBODY) != SIP_OPT_NOPARSEBODY)
			{
				if(g->pContentTypeHdr == SIP_NULL)
				{
					/* Body present but no content-Type specified */
					sip_error(SIP_Minor,(SIP_S8bit *)\
							"Message contains a body but no Content-type \
							header");
					*err = E_PARSER_ERROR;
					fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *) \
							dParserParams.pGCList,&dErr);
					fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *) \
							dParserParams.pError,&dErr);
					if(encoutbuffer != SIP_NULL)
						sip_freeString(encoutbuffer);
					if ((opt->dOption & SIP_OPT_BADMESSAGE)== \
							SIP_OPT_BADMESSAGE)
					{
						dParserParams.pSipMessage->dEntityErrorCount++;
						*err=E_PARSER_ERROR;
						*ppSipMessage=(dParserParams.pSipMessage);
						return SipSuccess;
					}
					else
					{
						sip_freeSipMessage((dParserParams.pSipMessage));
						return SipFail;
					}
				}
			}

			if(glbSipParserParseBody(0,opt, g->pContentTypeHdr,\
						&messagebody[bodystartindex],clen,\
						&((dParserParams.pSipMessage)->slMessageBody),err) \
					==SipFail)
			{
				if (*err!=E_NO_MEM)
					*err=E_PARSER_ERROR;
				fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
						dParserParams.pGCList,&dErr);
				fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)\
						dParserParams.pError,&dErr);
				if(encoutbuffer != SIP_NULL)
					sip_freeString(encoutbuffer);
				if (*err!=E_NO_MEM)
				{
					if ((opt->dOption & SIP_OPT_BADMESSAGE)==\
							SIP_OPT_BADMESSAGE)
					{
						dParserParams.pSipMessage->dEntityErrorCount++;
						*err=E_PARSER_ERROR;
						*ppSipMessage=(dParserParams.pSipMessage);
						return SipSuccess;
					}
					else
					{
						sip_freeSipMessage((dParserParams.\
									pSipMessage));
						return SipFail;
					}
				}
				else
				{
					sip_freeSipMessage((dParserParams.\
								pSipMessage));
					return SipFail;
				}
			}
		}
	}
	if(encoutbuffer != SIP_NULL)
		sip_freeString(encoutbuffer);
	*(ppSipMessage)=dParserParams.pSipMessage;
	fast_memfree(DECODE_MEM_ID,(SIP_Pvoid *)dParserParams.pGCList,\
			err);
	fast_memfree(DECODE_MEM_ID,dParserParams.pError,err);
	SIPDEBUG("Message parsed successfully. Returning success.\n");
	return SipSuccess;
}

/***********************************************************************
 ** FUNCTION: getHeaderLineIndex
 ***********************************************************************
 ** Function used to split the message into headers and return an
 ** Index to the end of the header and index to the start of
 ** message body.
 **********************************************************************/

SipBool getHeaderLineIndex
#ifdef ANSI_PROTO
(SIP_S8bit *pMesg, SIP_U32bit dMesgLen,SIP_U32bit *pHeaderEndIndex, \
 SIP_U32bit *pLineEndIndex, SIP_U32bit *pBodyStartIndex, SipUnknownHeader *pHeader, \
 SIP_U32bit *pMsgBodyFlag, SipError *pErr)
#else
(pMesg, dMesgLen, pLineEndIndex, pHeaderEndIndex, pBodyStartIndex, pHeader, \
 pMsgBodyFlag, pErr)
SIP_S8bit *pMesg;
SIP_U32bit dMesgLen;
SIP_U32bit *pLineEndIndex;
SIP_U32bit *pHeaderEndIndex;
SIP_U32bit *pBodyStartIndex;
SipUnknownHeader *pHeader;
SIP_U32bit *pMsgBodyFlag;
SipError *pErr;
#endif
{
	SIP_U32bit i=0, dHeaderStart=0,dNext=0;
	SIP_U32bit remLength=0;
	SIP_U32bit headerBodyFilled=0;

	SIP_U32bit CharSet[SIP_ASCII_CHAR_SET] = {\
		0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
			0,0,0,0,0,0};


	dHeaderStart= i;
	remLength = dMesgLen - (*pHeaderEndIndex);
	while ( i < remLength)
	{
		if (  CharSet[(unsigned char)pMesg[i]] == 0)
		{
			i++;
			continue;
		}

		if ( pMesg[i] == ':')
		{
			SIP_U32bit nonToken=0, dIndex=0;

			/* Found Colon; Fill header name */

			if ((pHeader->pName = (SIP_S8bit *) fast_memget(DECODE_MEM_ID,\
							(i-dHeaderStart)+2, pErr)) == SIP_NULL)
			{
				*pErr = E_NO_MEM;
				return SipFail;
			}

			memcpy(pHeader->pName,\
					&pMesg[dHeaderStart],(i-dHeaderStart)+1);
			*((pHeader->pName)+(i-dHeaderStart)+1) = '\0';

			/* check for LWS in header name code starts here */
			while(pHeader->pName[dIndex] != ':')
			{
				if( (pHeader->pName[dIndex] ==' ')  || \
						(pHeader->pName[dIndex] =='\t') || \
						(pHeader->pName[dIndex] =='\r') || \
						(pHeader->pName[dIndex] =='\n'))
				{
					nonToken=dIndex;
					dIndex++;
					break;
				}
				dIndex++;
			}
			if ( nonToken != 0)
			{
				while(pHeader->pName[dIndex] != ':')
				{
					if( (pHeader->pName[dIndex] !=' ')  && \
							(pHeader->pName[dIndex] !='\t') && \
							(pHeader->pName[dIndex] !='\r') && \
							(pHeader->pName[dIndex] !='\n'))
					{
						glbSipParserHeadererror("Incorrect LWS\n");
						*pErr = E_PARSER_ERROR;
						return SipFail;
					}
					dIndex++;
				}
				pHeader->pName[nonToken]=':';
				pHeader->pName[nonToken+1]='\0';
			}
			/* check for LWS in header name code ends here */

			i++;

		}
		else if ( i+1 > remLength)
		{
			glbSipParserHeadererror("End of the buffer reached\n");
			*pErr = E_NO_MEM;
			return SipFail;
		}
		else
		{
			if (isLineFold(&pMesg[i], &dNext) == SipSuccess)
			{
				if ( (i+dNext) > remLength)
				{
					glbSipParserHeadererror("End of the buffer reached\n");
					*pErr = E_NO_MEM;
					return SipFail;
				}
				else
				{
					/* Found Linefold; skip it and continue */
					i+=dNext;
					continue;
				}
			}
			else
			{
				glbSipParserHeadererror("Found header without colon\n");
				*pErr = E_PARSER_ERROR;
				return SipFail;
			}
		}

		dHeaderStart =i;
		while ( i < remLength )
		{
			if (  (CharSet[(unsigned char)pMesg[i]] == 0) || (pMesg[i] == ':'))
			{
				i++;
				continue;
			}
			if ( isLineFold(&pMesg[i], &dNext) == SipSuccess)
			{
				if ( (i+dNext) > remLength)
				{
					glbSipParserHeadererror("End of the buffer reached\n");
					*pErr = E_NO_MEM;
					return SipFail;
				}
				else
				{
					/* Found Linefold; skip it and continue */
					i+=dNext;
					continue;
				}
			}
			else if ( i+1 > remLength)
			{
				glbSipParserHeadererror("End of the buffer reached\n");
				*pErr = E_NO_MEM;
				return SipFail;
			}
			else
			{
				/*
				 * Fill in the header body & append to list of headers
				 */
				if ((pHeader->pBody = (SIP_S8bit*) fast_memget(DECODE_MEM_ID,\
								(i-dHeaderStart)+1, pErr)) == SIP_NULL)
				{
					*pErr = E_NO_MEM;
					return SipFail;
				}

				memcpy(pHeader->pBody,\
						&pMesg[dHeaderStart],(i-dHeaderStart));
				*((pHeader->pBody)+(i-dHeaderStart)) = '\0';

				/*
				 * header name and body ends up,
				 * fill the header end index
				 */
				if((pMesg[i+1]=='\n')||( pMesg[i+1]=='\r'))
					*pHeaderEndIndex += i+2;
				else
				{
					*pHeaderEndIndex += i+1;
#ifdef SIP_INCOMPLETE_MSG_CHECK
					if (i+1 >= remLength)
					{
						*pErr = E_INCOMPLETE;
						return SipFail;
					}
#endif
				}

				headerBodyFilled = 1;

				/*
				 * Found CR /LF which is not line fold
				 * could be next header or message body
				 * break out.
				 */

				break;
			}
		}

		if (headerBodyFilled !=1)
		{
#ifdef SIP_INCOMPLETE_MSG_CHECK
			{
				*pErr = E_INCOMPLETE;
				return SipFail;
			}
#else
			/*
			 * Fill in the header body & append to list of headers
			 */
			if ((pHeader->pBody = (SIP_S8bit*) fast_memget(DECODE_MEM_ID,\
							(i-dHeaderStart)+1, pErr)) == SIP_NULL)
			{
				*pErr = E_NO_MEM;
				return SipFail;
			}

			memcpy(pHeader->pBody,\
					&pMesg[dHeaderStart],(i-dHeaderStart));
			*((pHeader->pBody)+(i-dHeaderStart)) = '\0';

			/*
			 * header name and body ends up,
			 * fill the header end index
			 */
			if ((i+1)<=remLength)
			{
				if(pMesg[i+1]=='\n')
					*pHeaderEndIndex += i+2;
				else
					*pHeaderEndIndex += i+1;
			}

			else
			{
				*pHeaderEndIndex += i+1;

			}
			*pErr = E_NO_ERROR;
			return SipSuccess;

#endif
		}

		/* this is needed for dblnullbuffer option */

		*pLineEndIndex = *pHeaderEndIndex;

		/*
		 * If message body, return index to start of message body
		 * in buffer
		 */
		if ( ((pMesg[i] == '\r' )&& ( pMesg[i+1] == '\r'  ))
				||( (pMesg[i] == '\n' )  && ( pMesg[i+1] == '\n'  )))
		{
			if (((*pHeaderEndIndex)+1) < dMesgLen)
			{
				*pMsgBodyFlag = 1;
				*pBodyStartIndex = *pHeaderEndIndex;
			}
			*pLineEndIndex -= 2;
			*pErr = E_NO_ERROR;
			return SipSuccess;
		}
		else if (( (pMesg[i] == '\r') && ( pMesg[i+1] == '\n'  ) &&
					(pMesg[i+2] == '\r' )&& ( pMesg[i+3] == '\n'  )))
		{
			if (((*pHeaderEndIndex)+1) < dMesgLen)
			{
				*pMsgBodyFlag = 1;
				*pBodyStartIndex = (*pHeaderEndIndex)+2;
			}
			*pLineEndIndex -= 2;
			*pErr = E_NO_ERROR;
			return SipSuccess;
		}
		else
		{
			/* Next header..skip CR , LF */

			while (pMesg[i] == '\r' || pMesg[i] == '\n')
			{
				i++;
#ifdef SIP_INCOMPLETE_MSG_CHECK
				if (i >= remLength)
				{
					*pErr = E_INCOMPLETE;
					return SipFail;
				}
#endif
				*pLineEndIndex -= 1;
			}
			dHeaderStart=i;
			*pErr = E_NO_ERROR;
			return SipSuccess;
		}
	}

#ifdef SIP_INCOMPLETE_MSG_CHECK	
	*pErr = E_INCOMPLETE;
#endif	
	return SipFail;

}

/***********************************************************************
 ** FUNCTION: isLineFold
 ***********************************************************************
 ** Function checks if buffer starts with line fold, returns index
 ** to next token after linefold
 **********************************************************************/
SipBool isLineFold
#ifdef ANSI_PROTO
(SIP_S8bit *pBuf, SIP_U32bit *pNext)
#else
(pBuf, pNext)
	SIP_S8bit *pBuf;
	SIP_U32bit *pNext;
#endif
{
	if ( (((*(pBuf) == '\r') ||( *(pBuf ) == '\n')) && \
				((*(pBuf + 1) == ' ') || (*(pBuf  + 1) == '\t'))))
	{
		*pBuf=' ';
		*pNext=2;
		return SipSuccess;
	}
	else if ((((*(pBuf) == '\r') && (*(pBuf + 1) == '\n')) && \
				((*(pBuf + 2) == ' ') || (*(pBuf + 2) == '\t'))))
	{
		*pBuf=' ';
		*(pBuf + 1)=' ';
		*pNext=3;
		return SipSuccess;
	}
	else
		return SipFail;

}

/***********************************************************************
 ** FUNCTION: removeLineFold
 ***********************************************************************
 ** Function checks if buffer contains a  line fold, and if so replaces
 ** the occurrence of the /r/n with spaces
 **********************************************************************/
void removeLineFold
#ifdef ANSI_PROTO
(SIP_S8bit *pBuf)
#else
(pBuf)
	SIP_S8bit *pBuf;
#endif
{
	const SIP_S8bit *search="\r\n";
	SIP_S8bit *pLocation=SIP_NULL;
	SipBool foundLineFold=SipSuccess;

	while(foundLineFold)
	{
		pLocation=sip_strstr(pBuf,(SIP_S8bit *)search);
		if (pLocation)
		{
			if ((pLocation[2]=='\t')||(pLocation[2]==' '))
			{
				/*A line fold has been detected..REMOVE IT*/
				pLocation[0]=' ';
				pLocation[1]=' ';
			}
			pBuf=pLocation+2;
			foundLineFold=SipSuccess;
		}
		else
			foundLineFold=SipFail;
	}
}

/*****************************************************************************
 ** FUNCTION:
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserTokenserror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in tokenparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Token related Header\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserTokensltokenerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserTokensltokenerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	/*      SipError err;*/
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in tokensltokenparse\n");
	sip_error (SIP_Minor,\
			"Syntax error while parsing Token/Token related Header\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserDatetimeerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserDatetimeerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	sip_error (SIP_Minor, \
			"Syntax error while parsing Date/RetryAfter/Expires/TimeStamp\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in datetimeparse\n");
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserUtf8error
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserUtf8error
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in utf8parse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Subject/User Agent\n");
	sip_error(SIP_Minor, s);
	return 1;
}

#undef HSS_SIP_INCLUDE_IMPP_PRES
#ifdef SIP_IMPP
#define HSS_SIP_INCLUDE_IMPP_PRES 1
#endif
#ifdef SIP_PRES
#define HSS_SIP_INCLUDE_IMPP_PRES 1
#endif

#ifdef HSS_SIP_INCLUDE_IMPP_PRES
/***********************************************************************
 ** FUNCTION: glbSipParserImerror
 ***********************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ***********************************************************************/
int glbSipParserImerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Imparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Im-Url or Pres-Url\n");
	sip_error(SIP_Minor, s);
	return 1;
}

#endif

/*****************************************************************************
 ** FUNCTION: glbSipParserFromtoerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserFromtoerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in fromtoparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing From/To/related\n");
	sip_error(SIP_Minor, s);
	return 1;
}
/*****************************************************************************
 ** FUNCTION: glbSipParserToerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserToerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Toparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing To related\n");
	sip_error(SIP_Minor, s);
	return 1;
}
#ifdef SIP_DCS
/*****************************************************************************
 ** FUNCTION: glbSipParserDcserror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserDcserror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy = s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Dcsparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Dcs/related\n");
	sip_error(SIP_Minor, s);
	return 1;
}
#endif
/*****************************************************************************
 ** FUNCTION: glbSipParserContacterror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserContacterror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy =s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in contactparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Contact/Callid/Also/Replaces/ReplyTo\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserPgperror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserPgperror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in pgpparse\n");
	sip_error (SIP_Minor, "Syntax error while Authenticate/Authorisation\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserViaerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserViaerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in vaiparse\n");
	sip_error(SIP_Minor, s);
	sip_error (SIP_Minor, "Syntax error while parsing Via Header\n");
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserKeyerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserKeyerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in keyparse\n");
	sip_error (SIP_Minor,"Syntax error while parsing Resp-Key or Encryption\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserTokencommenterror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserTokencommenterror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in tokencommentparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Comment\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserHeadererror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserHeadererror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in headerparse\n");
	sip_error (SIP_Minor, "Syntax error while splitting into headers\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserStatuslineerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserStatuslineerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in statuslineparse\n");
	sip_error (SIP_Minor, "Syntax error while splitting Status line\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserReqlineerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserReqlineerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in reqlineparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Request Line\n");
	sip_error(SIP_Minor, s);

	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserMediaerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserMediaerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
const char *s;
#endif
{
        const char *dummy;
        dummy=s;
        SIPDEBUG("SIP DECODE MESSAGE Syntax error in mediaparse\n");
        sip_error (SIP_Minor, "Syntax error while parsing Media Parse\n");
        sip_error(SIP_Minor, s);
        return 1;
}
#ifdef SIP_MWI
/*****************************************************************************
 ** FUNCTION: glbSipParserMesgSummaryerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserMesgSummaryerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Message Waiting Msg Body parse\n");
	sip_error (SIP_Minor, "Syntax error while parsing SIP_MWI\n");
	sip_error(SIP_Minor, s);
	return 1;
}
#endif
/*****************************************************************************
 ** FUNCTION: glbSipParserSdperror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserSdperror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in sdpparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing SDP\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserAttriberror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserAttriberror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;

	sip_error (SIP_Minor, "Syntax error while parsing Attribute (SDP)\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in attribparse\n");
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserMimeerror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserMimeerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;

	sip_error (SIP_Minor, "Syntax error while parsing MIME headers\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in mimeparse\n");
	return 1;
}

/*****************************************************************************
 ** FUNCTION: glbSipParserRprTokenserror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserRprTokenserror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;

	dummy=s;
	sip_error (SIP_Minor, "Syntax error while parsing RPR Token headers\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in rprtokensparse\n");
	return 1;
}

#ifdef SIP_CCP
/*****************************************************************************
 ** FUNCTION: glbSipParserRejectContacterror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserRejectContacterror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;

	dummy=s;
	sip_error (SIP_Minor,"Syntax error while parsing Reject Contact headers\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in rejectcontactparse\n");
	return 1;
}
/*****************************************************************************
 ** FUNCTION: glbSipParserAcceptContacterror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserAcceptContacterror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;

	dummy=s;
	sip_error (SIP_Minor,"Syntax error while parsing Accept Contact headers\n");
	sip_error(SIP_Minor, s);
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in acceptcontactparse\n");
	return 1;
}
#endif /* of CCP */

#ifdef SIP_3GPP
/*****************************************************************************
 ** FUNCTION: glbSipParser3gpperror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParser3gpperror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in 3gppparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing 3gpp related\n");
	sip_error(SIP_Minor, s);
	return 1;
}
#endif

#ifdef SIP_CONGEST
/*****************************************************************************
 ** FUNCTION: glbSipParserCongesterror
 *****************************************************************************
 ** Function required by the yacc generated parser to handle parse errors.
 ****************************************************************************/
int glbSipParserCongesterror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
	const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Congestparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Congest related\n");
	sip_error(SIP_Minor, s);
	return 1;
}
#endif

#ifdef __cplusplus
}
#endif

