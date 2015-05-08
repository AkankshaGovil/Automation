/*
** THIS FILE NEED NOT BE DIRECTLY INCLUDED BY THE USER
*/

/**************************************************************************
** FUNCTION:
**  This file contains the prototypes of some internal lex/yacc functions
**
***************************************************************************
**
** FILENAME:
**  sipparserinc.h
**
** DESCRIPTION
**
**
**  DATE           NAME                     REFERENCE
** 17Nov99  	KSBinu, Arjun RC			Initial
**
**
** Copyright 1999, Hughes Software Systems, Ltd.
***************************************************************************/

#ifndef __SIP_PARSER_INC_H_
#define __SIP_PARSER_INC_H_

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "siptrace.h"

#ifdef SIP_CCP
#include "ccpstruct.h"
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


#define YYERROR_VERBOSE

/* Prototypes of all lexers */
/*
#ifdef SIP_CCP
extern int glbSipParserAcceptContactlex(void);
extern int glbSipParserRejectContactlex(void);
#endif
extern int glbSipParserAttriblex(void);
extern int glbSipParserContactlex(void);
extern int glbSipParserDatetimelex(void);
extern int glbSipParserFixedlex(void);
extern int glbSipParserFromtolex(void);
extern int glbSipParserHeaderlex(void);
extern int glbSipParserKeylex(void);
extern int glbSipParserMedialex(void);
extern int glbSipParserMimelex(void);
extern int glbSipParserPgplex(void);
extern int glbSipParserReqlinelex(void);
extern int glbSipParserRprTokenslex(void);
extern int glbSipParserSdplex(void);
extern int glbSipParserStatuslinelex(void);
extern int glbSipParserTokencommentlex(void);
extern int glbSipParserTokenslex(void);
extern int glbSipParserTokensltokenlex(void);
extern int glbSipParserUtf8lex(void);
extern int glbSipParserVialex(void);
#ifdef SIP_DCS
extern int glbSipParserDcslex(void);
#endif
*/


/* Prototypes of all error handlers required in the bison files */
#ifdef SIP_CCP
extern int glbSipParserAcceptContacterror(const char *s);
extern int glbSipParserRejectContacterror(const char *s);
#endif
extern int glbSipParserAttriberror(const char *s);
extern int glbSipParserContacterror(const char *s);
extern int glbSipParserDatetimeerror(const char *s);
extern int glbSipParserFixederror(const char *s);
extern int glbSipParserFromtoerror(const char *s);
extern int glbSipParserToerror(const char *s);
extern int glbSipParserHeadererror(const char *s);
extern int glbSipParserKeyerror(const char *s);
extern int glbSipParserMediaerror(const char *s);
extern int glbSipParserMimeerror(const char *s);
extern int glbSipParserPgperror(const char *s);
extern int glbSipParserReqlineerror(const char *s);
extern int glbSipParserRprTokenserror(const char *s);
extern int glbSipParserSdperror(const char *s);
extern int glbSipParserStatuslineerror(const char *s);
extern int glbSipParserTokencommenterror(const char *s);
extern int glbSipParserTokenserror(const char *s);
extern int glbSipParserTokensltokenerror(const char *s);
extern int glbSipParserUtf8error(const char *s);
extern int glbSipParserViaerror(const char *s);
extern int glbSipParserImerror(const char *s);
#ifdef SIP_MWI
extern int glbSipParserMesgSummaryerror(const char *s);
#endif
#ifdef SIP_DCS
extern int glbSipParserDcserror(const char *s);
#endif
#ifdef SIP_3GPP
extern int glbSipParser3gpperror(const char *s);
#endif
#ifdef SIP_CONGEST
extern int glbSipParserCongesterror(const char *s);
#endif

#ifdef SIP_DCS
extern void sip_lex_Dcs_reset_state(void );
#endif
extern void sip_lex_Contact_reset_state(void );
#ifdef SIP_CCP
extern void sip_lex_AcceptContact_reset_state(void);
extern void sip_lex_RejectContact_reset_state(void);
#endif
extern void sip_lex_Via_reset_state(void );
extern void sip_lex_Mime_reset_state(void );
extern void sip_lex_Fromto_reset_state(void );
extern void sip_lex_To_reset_state(void );
extern void sip_lex_Via_reset_state(void );
extern void sip_lex_Pgp_reset_state(void );
extern void sip_lex_Sdp_reset_state(void);
extern void sip_lex_Reqline_reset_state(void);
#ifdef SIP_3GPP
extern void sip_lex_3gpp_reset_state(void );
#endif
#ifdef SIP_CONGEST
extern void sip_lex_congest_reset_state(void );
#endif

extern void sip_lex_Sdp_begin_uric(void);
extern void sip_lex_Sdp_begin_uri_allow(void);


extern int 			glbSdpParserState;
extern int 			glbSdpParserRepeatState;
extern SIP_S8bit*		glbSipParserInput;

/* Mutex used by the lexer */
#ifdef SIP_THREAD_SAFE
extern synch_id_t	glbLexTokensltokenMutex;
extern synch_id_t	glbLexTokensMutex;
extern synch_id_t	glbLexContactMutex;
extern synch_id_t	glbLexDatetimeMutex;
extern synch_id_t	glbLexKeyMutex;
extern synch_id_t	glbLexFromtoMutex;
extern synch_id_t	glbLexViaMutex;
extern synch_id_t	glbLexPgpMutex;
extern synch_id_t	glbLexUtf8Mutex;
extern synch_id_t	glbLexTokencommentMutex;
extern synch_id_t	glbLexAttribMutex;
extern synch_id_t	glbLexHeaderMutex;
extern synch_id_t	glbLexMediaMutex;
extern synch_id_t	glbLexMimeMutex;
extern synch_id_t	glbLexReqlineMutex;
extern synch_id_t	glbLexSdpMutex;
extern synch_id_t	glbLexStatusMutex;
extern synch_id_t	glbLexKeyMutex;
extern synch_id_t	glbLexTelMutex;
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


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
