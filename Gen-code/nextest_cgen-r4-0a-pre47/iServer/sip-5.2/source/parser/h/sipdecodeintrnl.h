/*
** THIS FILE IS INTERNALLY INCLUDED BY THE STACK
** USER SHOULD NOT INCLUDE THIS FILE DIRECTLY
*/

/**************************************************************************
** FUNCTION:
**  This file contains the INTERNAL prototypes of the primary decode
**	entry point
**
***************************************************************************
**
** FILENAME:
**  sipdecodeinternal.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** ------         -------                     ----------
** 17Nov99  	KSBinu, Arjun RC			 Initial
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/



#ifndef __SIP_DECODEINTRNL_H_
#define __SIP_DECODEINTRNL_H_

#include <stdlib.h>
#include "siplist.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	SIP_S8bit 	*pToken;
	SIP_U16bit 	dLength;
	SIP_S8bit 	dChar;
} SipTokenBuffer;

typedef struct
{
	SipHeaderParserParam	*pParserStruct;
	SipHdrTypeList			*pList;
	SIP_S8bit				ignoreErrors;
	SipBool					noSelectiveCallBacks;
} SipParseEachHeaderParam;

typedef struct
{
	SipSdpParserParam	*pSdpParam;
	int 				parserState;
	int					repeatState;
} SipParseEachSdpLineParam;

extern void (*glbSipParserReleaseBuffer[HEADERTYPENUM])(void);
extern int (*glbSipParserScanBuffer[HEADERTYPENUM])(SIP_S8bit *, SIP_U32bit);
extern int (*glbSipParserLexer[HEADERTYPENUM])(SipTokenBuffer *);
extern void (*glbSipParserReset[HEADERTYPENUM])(void);
extern int (*glbSipParserParser[HEADERTYPENUM])(void *pParserStruct);
#ifdef YYSTYPE
#undef YYSTYPE
#endif

#define YYSTYPE SipTokenBuffer

#define PASSTOKEN(x) \
{ \
	lval->pToken = (x); \
	lval->dLength = strlen(x); \
}

#define PASSCHAR(x) \
{ \
	lval->dChar = (x); \
}


extern SIP_S8bit * sip_tokenBufferDup _ARGS_((SipTokenBuffer dTokenBuffer));
extern void glbSipParserHeaderNameStrip _ARGS_((SIP_Pvoid data,\
	SIP_Pvoid pParserStruct));

extern void sip_getTypeFromName _ARGS_((SIP_S8bit *pName, en_HeaderType *pType, SipError *pError));

extern void glbSipParserParseEachHeader _ARGS_((SipUnknownHeader *pHeader, \
	SIP_S8bit *pHeaderBuffer, SIP_U32bit scanLength, SipOptions *pOpt, \
	SIP_Pvoid pParserStruct));

SipBool getHeaderLineIndex _ARGS_((SIP_S8bit *pMesg, SIP_U32bit dMesgLen, SIP_U32bit *pLineEndIndex, SIP_U32bit *pHeaderEndIndex,SIP_U32bit *pBodyStartIndex, SipUnknownHeader *pHeader, SIP_U32bit *pMsgBodyFlag, SipError *pErr));

SipBool isLineFold _ARGS_((SIP_S8bit *pBuf, SIP_U32bit *pNext));

void removeLineFold _ARGS_ ((SIP_S8bit *pBuf));

extern void glbSipParserSdpHeaderParse _ARGS_((SIP_Pvoid data, SIP_Pvoid pParam));

extern SipBool glbSipParserDecodeMessage _ARGS_((SIP_S8bit *message, \
	SipOptions *opt, SIP_U32bit messageLength, SIP_S8bit **nextmesg,\
	SipEventContext *pContext,SipMessage **ppMessage, SipError *err) );

extern int glbSipParserTokenserror _ARGS_((const char *s));
extern int glbSipParserTokensltokenerror _ARGS_((const char *s));
extern int glbSipParserDatetimeerror _ARGS_((const char *s));
extern int glbSipParserUtf8error _ARGS_((const char *s));
extern int glbSipParserFromtoerror _ARGS_((const char *s));
extern int glbSipParserToerror _ARGS_((const char *s));
extern int glbSipParserContacterror _ARGS_((const char *s));
extern int glbSipParserPgperror _ARGS_((const char *s));
extern int glbSipParserFixederror _ARGS_((const char *s));
extern int glbSipParserViaerror _ARGS_((const char *s));
extern int glbSipParserKeyerror _ARGS_((const char *s));
extern int glbSipParserTokencommenterror _ARGS_((const char *s));
extern int glbSipParserHeadererror _ARGS_((const char *s));
extern int glbSipParserStatuslineerror _ARGS_((const char *s));
extern int glbSipParserReqlineerror _ARGS_((const char *s));
extern int glbSipParserMediaerror _ARGS_((const char *s));
extern int glbSipParserSdperror _ARGS_((const char *s));
extern int glbSipParserAttriberror _ARGS_((const char *s));
#ifdef SIP_CCP
extern int glbSipParserAcceptContacterror _ARGS_((const char *s));
extern int glbSipParserRejectContacterror _ARGS_((const char *s));
#endif
extern int glbSipParserRprTokenserror _ARGS_((const char *s));
extern int glbSipParserMimeerror _ARGS_((const char *s));
#ifdef SIP_3GPP
extern int glbSipParser3gpperror _ARGS_((const char *s));
#endif

extern int glbSipParserTokensparse _ARGS_((void *pParserStruct));
extern int glbSipParserTokensltokenparse _ARGS_((void *pParserStruct));
extern int glbSipParserDatetimeparse _ARGS_((void *pParserStruct));
extern int glbSipParserUtf8parse _ARGS_((void *pParserStruct));
extern int glbSipParserFromtoparse _ARGS_((void *pParserStruct));
extern int glbSipParserToparse _ARGS_((void *pParserStruct));
extern int glbSipParserContactparse _ARGS_((void *pParserStruct));
extern int glbSipParserPgpparse _ARGS_((void *pParserStruct));
extern int glbSipParserFixedparse _ARGS_((void *pParserStruct));
extern int glbSipParserViaparse _ARGS_((void *pParserStruct));
extern int glbSipParserKeyparse _ARGS_((void *pParserStruct));
extern int glbSipParserTokencommentparse _ARGS_((void *pParserStruct));
extern int glbSipParserHeaderparse _ARGS_((void *pParserStruct));
extern int glbSipParserStatuslineparse _ARGS_((void *pParserStruct));
extern int glbSipParserReqlineparse _ARGS_((void *pParserStruct));
extern int glbSipParserMediaparse _ARGS_((void *pParserStruct));
extern int glbSipParserSdpparse _ARGS_((void *pParserStruct));
#ifdef SIP_MWI
extern int glbSipParserMesgSummaryparse _ARGS_((void *pParserStruct));
#endif
extern int glbSipParserAttribparse _ARGS_((void *pParserStruct));
extern int glbSipParserMimeparse _ARGS_((void *pParserStruct));
#ifdef SIP_CCP
extern int glbSipParserAcceptContactparse _ARGS_((void *pParserStruct));
extern int glbSipParserRejectContactparse _ARGS_((void *pParserStruct));
#endif
extern int glbSipParserRprTokensparse _ARGS_((void *pParserStruct));

#ifdef SIP_DCS
extern int glbSipParserDcsparse _ARGS_ ((void *pParserStruct));


#endif
#ifdef SIP_3GPP
extern int glbSipParser3gppparse _ARGS_((void *pParserStruct));
#endif
#ifdef SIP_CONGEST
extern int glbSipParserCongestparse _ARGS_((void *pParserStruct));
#endif


extern int glbSipParserTokenslex _ARGS_((YYSTYPE *));
extern int glbSipParserTokensltokenlex _ARGS_((YYSTYPE *));
extern int glbSipParserDatetimelex _ARGS_((YYSTYPE *));
extern int glbSipParserUtf8lex _ARGS_((YYSTYPE *));
extern int glbSipParserFromtolex _ARGS_((YYSTYPE * ));
extern int glbSipParserTolex _ARGS_((YYSTYPE * ));
extern int glbSipParserContactlex _ARGS_((YYSTYPE *));
extern int glbSipParserPgplex _ARGS_((YYSTYPE *));
extern int glbSipParserFixedlex _ARGS_((YYSTYPE *));
extern int glbSipParserVialex _ARGS_((YYSTYPE *));
extern int glbSipParserKeylex _ARGS_((YYSTYPE *));
extern int glbSipParserTokencommentlex _ARGS_((YYSTYPE *));
extern int glbSipParserHeaderlex _ARGS_((YYSTYPE *));
extern int glbSipParserStatuslinelex _ARGS_((YYSTYPE *));
extern int glbSipParserReqlinelex _ARGS_((YYSTYPE *));
extern int glbSipParserMedialex _ARGS_((YYSTYPE *));
extern int glbSipParserSdplex _ARGS_((YYSTYPE *));
#ifdef SIP_MWI
extern int glbSipParserMesgSummarylex _ARGS_((YYSTYPE *));
extern void sip_lex_MesgSummary_reset_state(void);
#endif
extern int glbSipParserAttriblex _ARGS_((YYSTYPE *));
#ifdef SIP_CCP
extern int glbSipParserAcceptContactlex _ARGS_((YYSTYPE *));
extern int glbSipParserRejectContactlex _ARGS_((YYSTYPE *));
#endif
extern int glbSipParserRprTokenslex _ARGS_((YYSTYPE *));
extern int glbSipParserMimelex _ARGS_((YYSTYPE *));
#ifdef SIP_3GPP
extern int glbSipParser3gpplex _ARGS_((YYSTYPE *));
#endif
#ifdef SIP_CONGEST
extern int glbSipParserCongestlex _ARGS_((YYSTYPE *));
#endif


extern int glbSipParserTokenslexreal _ARGS_((YYSTYPE *));
extern int glbSipParserTokensltokenlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserDatetimelexreal _ARGS_((YYSTYPE *));
extern int glbSipParserUtf8lexreal _ARGS_((YYSTYPE *));
extern int glbSipParserFromtolexreal _ARGS_((YYSTYPE * ));
extern int glbSipParserTolexreal _ARGS_((YYSTYPE * ));
extern int glbSipParserContactlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserPgplexreal _ARGS_((YYSTYPE *));
extern int glbSipParserFixedlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserVialexreal _ARGS_((YYSTYPE *));
extern int glbSipParserKeylexreal _ARGS_((YYSTYPE *));
extern int glbSipParserTokencommentlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserHeaderlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserStatuslinelexreal _ARGS_((YYSTYPE *));
extern int glbSipParserReqlinelexreal _ARGS_((YYSTYPE *));
extern int glbSipParserMedialexreal _ARGS_((YYSTYPE *));
extern int glbSipParserSdplexreal _ARGS_((YYSTYPE *));
extern int glbSipParserAttriblexreal _ARGS_((YYSTYPE *));
#ifdef SIP_MWI
extern int glbSipParserMesgSummarylexreal _ARGS_((YYSTYPE *));
#endif
#ifdef SIP_CCP
extern int glbSipParserAcceptContactlexreal _ARGS_((YYSTYPE *));
extern int glbSipParserRejectContactlexreal _ARGS_((YYSTYPE *));
#endif
extern int glbSipParserRprTokenslexreal _ARGS_((YYSTYPE *));
extern int glbSipParserMimelexreal _ARGS_((YYSTYPE *));


#ifdef SIP_DCS
extern int glbSipParserDcserror _ARGS_((const char *s));/*DCS*/

extern int glbSipParserDcslex _ARGS_((YYSTYPE *));
extern int glbSipParserDcslexreal _ARGS_((YYSTYPE *));
#endif

#ifdef SIP_3GPP
extern int glbSipParser3gpplexreal _ARGS_((YYSTYPE *));
#endif
#ifdef SIP_3GPP
extern int glbSipParserCongestlexreal _ARGS_((YYSTYPE *));
#endif

#ifdef SIP_CCP
extern void sip_lex_AcceptContact_free _ARGS_((void));
extern void sip_lex_RejectContact_free _ARGS_((void));
#endif
extern void sip_lex_Attrib_free _ARGS_((void));
extern void sip_lex_Contact_free _ARGS_((void));
extern void sip_lex_Datetime_free _ARGS_((void));
extern void sip_lex_Fixed_free _ARGS_((void));
extern void sip_lex_Fromto_free _ARGS_((void));
extern void sip_lex_To_free _ARGS_((void));
extern void sip_lex_Header_free _ARGS_((void));
extern void sip_lex_Key_free _ARGS_((void));
extern void sip_lex_Media_free _ARGS_((void));
extern void sip_lex_Mime_free _ARGS_((void));
extern void sip_lex_Pgp_free _ARGS_((void));
extern void sip_lex_Reqline_free _ARGS_((void));
extern void sip_lex_RprTokens_free _ARGS_((void));
extern void sip_lex_Sdp_free _ARGS_((void));
#ifdef SIP_MWI
extern void sip_lex_MesgSummary_free _ARGS_((void));
#endif
extern void sip_lex_Statusline_free _ARGS_((void));
extern void sip_lex_Tokencomment_free _ARGS_((void));
extern void sip_lex_Tokens_free _ARGS_((void));
extern void sip_lex_Tokensltoken_free _ARGS_((void));
extern void sip_lex_Utf8_free _ARGS_((void));
extern void sip_lex_Via_free _ARGS_((void));

#ifdef SIP_DCS
extern void sip_lex_Dcs_free _ARGS_((void));/*DCS*/
#endif

#ifdef SIP_3GPP
extern void sip_lex_3gpp_free _ARGS_((void));
#endif

#ifdef SIP_CONGEST
extern void sip_lex_congest_free _ARGS_((void));
#endif

#ifdef SIP_CCP
extern int sip_lex_AcceptContact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_RejectContact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
#endif

extern int sip_lex_Attrib_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Contact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Datetime_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Fixed_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Fromto_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_To_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Header_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Key_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Media_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Mime_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Pgp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Reqline_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_RprTokens_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Sdp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
#ifdef SIP_MWI
extern int sip_lex_MesgSummary_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
#endif
extern int sip_lex_Statusline_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Tokencomment_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Tokens_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Tokensltoken_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Utf8_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
extern int sip_lex_Via_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);

#ifdef SIP_DCS
extern int sip_lex_Dcs_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);/*DCS*/

#endif

#ifdef SIP_3GPP
extern int sip_lex_3gpp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
#endif
#ifdef SIP_CONGEST
extern int sip_lex_congest_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength);
#endif


#ifdef SIP_CCP
extern void sip_lex_AcceptContact_release_buffer(void);
extern void sip_lex_RejectContact_release_buffer(void);
#endif

extern void sip_lex_Attrib_release_buffer(void);
extern void sip_lex_Contact_release_buffer(void);
extern void sip_lex_Datetime_release_buffer(void);
extern void sip_lex_Fixed_release_buffer(void);
extern void sip_lex_Fromto_release_buffer(void);
extern void sip_lex_To_release_buffer(void);
extern void sip_lex_Header_release_buffer(void);
extern void sip_lex_Key_release_buffer(void);
extern void sip_lex_Media_release_buffer(void);
extern void sip_lex_Mime_release_buffer(void);
extern void sip_lex_Pgp_release_buffer(void);
extern void sip_lex_Reqline_release_buffer(void);
extern void sip_lex_RprTokens_release_buffer(void);
extern void sip_lex_Sdp_release_buffer(void);
#ifdef SIP_MWI
extern void sip_lex_MesgSummary_release_buffer(void);
#endif
extern void sip_lex_Statusline_release_buffer(void);
extern void sip_lex_Tokencomment_release_buffer(void);
extern void sip_lex_Tokens_release_buffer(void);
extern void sip_lex_Tokensltoken_release_buffer(void);
extern void sip_lex_Utf8_release_buffer(void);
extern void sip_lex_Via_release_buffer(void);

#ifdef SIP_DCS
extern void sip_lex_Dcs_release_buffer(void);/*DCS*/
#endif

#ifdef SIP_3GPP
extern void sip_lex_3gpp_release_buffer(void);/*3GPP*/
#endif
#ifdef SIP_CONGEST
extern void sip_lex_congest_release_buffer(void);/*Congest*/
#endif

#ifdef SIP_THREAD_SAFE
extern synch_id_t	glbLexTokensltokenMutex;
extern synch_id_t	glbLexTokensMutex;
extern synch_id_t	glbLexContactMutex;
extern synch_id_t	glbLexDatetimeMutex;
extern synch_id_t	glbLexFromtoMutex;
extern synch_id_t	glbLexToMutex;
extern synch_id_t	glbLexViaMutex;
extern synch_id_t	glbLexPgpMutex;
extern synch_id_t	glbLexUtf8Mutex;
extern synch_id_t	glbLexTokencommentMutex;
extern synch_id_t	glbLexAttribMutex;
extern synch_id_t	glbLexHeaderMutex;
extern synch_id_t	glbLexMediaMutex;
extern synch_id_t	glbLexMimeMutex;
#ifdef SIP_MWI
extern synch_id_t	glbLexMesgSummaryMutex;
#endif
extern synch_id_t	glbLexReqlineMutex; extern synch_id_t	glbLexSdpMutex;
extern synch_id_t	glbLexStatusMutex;
extern synch_id_t	glbLexKeyMutex;
extern synch_id_t	glbLexTelMutex;
extern synch_id_t	glbLexImMutex;
extern synch_id_t	glbLexImMutex;
#ifdef SIP_CCP
extern synch_id_t	glbLexAcceptContactMutex;
extern synch_id_t	glbLexRejectContactMutex;
#endif
extern synch_id_t	glbLexRprTokensMutex;
#ifdef SIP_DCS
extern synch_id_t	glbLexDcsMutex;
#endif
#ifdef SIP_3GPP
extern synch_id_t	glbLex3gppMutex;
#endif
#ifdef SIP_CONGEST
extern synch_id_t	glbLexCongestMutex;
#endif
#endif

extern SipBool sip_decodeInvokeTimerCallbacks \
	_ARGS_((SipMessage *tempMessage, SipEventContext *pContext, SipError *err));
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
