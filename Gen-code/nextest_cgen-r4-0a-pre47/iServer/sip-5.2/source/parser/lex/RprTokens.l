%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for rpr headers

 *********************************************************************
 **
 ** FILENAME:
 ** tokensltoken.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----	----	     		---------	       --------
 ** 24/11/99  KSBinu, Arjun RC			         Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "RprTokens.tab.h"
#define yylval glbSipParserRprTokenslval
#include "siplist.h"
#define YY_DECL int glbSipParserRprTokenslexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_RprTokens_fatal_error(msg)
static jmp_buf glbSipParserRprTokensJumpBuf;
void sip_lex_RprTokens_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}

UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`"|"#")+

LWS [ \t]*
RSEQ [Rr][Ss][Ee][Qq]{LWS}":"
RACK [Rr][Aa][Cc][Kk]{LWS}":"
%%
{LWS} ;
{RSEQ}	{
		SIPDEBUG("SIP PARSER LEXER RPRTOKENS Rseq:\n");
	 	return RSEQ;
	}
{RACK}	{
		SIPDEBUG("SIP PARSER LEXER RPRTOKENS rack:\n");
	 	return RACK;
	}
{DIGITS}	{
		SIPDEBUG("SIP PARSER LEXER RPRTOKENS digits\n");
		PASSTOKEN(yytext)
		return DIGITS;
	}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER RPRTOKENS Token\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
.|\n	{
		SIPDEBUG("SIP PARSER LEXER RPRTOKENS default\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}


void sip_lex_RprTokens_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_RprTokens_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexRprTokensMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserRprTokensJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_RprTokens_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexRprTokensMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_RprTokens_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexRprTokensMutex);
#endif

}

int glbSipParserRprTokenslex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserRprTokenslexreal(yylval));
#else
	if (setjmp(glbSipParserRprTokensJumpBuf) == 0)
	{
		return(glbSipParserRprTokenslexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexRprTokensMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_RprTokens_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserRprTokensJumpBuf,1);
}
#endif

