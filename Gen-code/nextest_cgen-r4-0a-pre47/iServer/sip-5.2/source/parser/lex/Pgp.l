
%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for pgp headers

 *********************************************************************
 **
 ** FILENAME:
 ** utf8.l
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
#include "Pgp.tab.h" 
#define yylval glbSipParserPgplval
#include "siplist.h"
#define YY_DECL int glbSipParserPgplexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Pgp_fatal_error(msg)
static jmp_buf glbSipParserPgpJumpBuf;
void sip_lex_Pgp_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}
%s AllowB64 AllowBasic
LWS [ \t]*
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
QSTRING "\""(([^"\\\n])|("\\".))*["]
BASE64CHAR ({ALPHANUM}|"+"|"/"|"=")
BASE64STRING {BASE64CHAR}+
COLON ":"
COMMA ","({LWS}",")*
TOKENEQUALS {TOKEN}{LWS}"="
COMMATOKENEQUALS ","{LWS}{TOKEN}{LWS}"="
BASIC [Bb][Aa][Ss][Ii][Cc] 
AUTHORIZATION [Aa][Uu][Tt][Hh][Oo][Rr][Ii][Zz][Aa][Tt][Ii][Oo][Nn]{LWS}{COLON}
PROXYAUTHORIZATION [Pp][Rr][Oo][Xx][Yy]-[Aa][Uu][Tt][Hh][Oo][Rr][Ii][Zz][Aa][Tt][Ii][Oo][Nn]{LWS}{COLON}
PROXYAUTHENTICATE [Pp][Rr][Oo][Xx][Yy]-[Aa][Uu][Tt][Hh][Ee][Nn][Tt][Ii][Cc][Aa][Tt][Ee]{LWS}{COLON} 
WWWAUTHENTICATE [Ww][Ww][Ww]-[Aa][Uu][Tt][Hh][Ee][Nn][Tt][Ii][Cc][Aa][Tt][Ee]{LWS}{COLON}
EQUALS {LWS}"="{LWS}
%%
{LWS} ;
<AllowB64>{BASE64STRING} {
		SIPDEBUG("SIP PARSER LEXER PGP base64string \n");
		PASSTOKEN(yytext)
		BEGIN AllowBasic;
		return BASE64STRING;
	}
{AUTHORIZATION}		{ 
		BEGIN AllowBasic;
		SIPDEBUG("SIP PARSER LEXER PGP authorization\n");
		return AUTHORIZATION;
	}
{PROXYAUTHORIZATION}	{
		SIPDEBUG("SIP PARSER LEXER PGP proxyauthorization\n");
		BEGIN AllowBasic;
		return PROXYAUTHORIZATION;
	}
{PROXYAUTHENTICATE}	{
		SIPDEBUG("SIP PARSER LEXER PGP proxyauthenticate\n");
		return PROXYAUTHENTICATE;
	}
{WWWAUTHENTICATE}	{
		SIPDEBUG("SIP PARSER LEXER PGP WWWAuthenicate\n");
		return WWWAUTHENTICATE;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER PGP EQUALS\n");
		return EQUALS;
	}
<AllowBasic>{BASIC} 		{
		SIPDEBUG("SIP PARSER LEXER PGP Basic\n");
		BEGIN AllowB64;
		PASSTOKEN(yytext)
		return BASIC;
	}
{COMMA}			{
	 	SIPDEBUG("SIP PARSER LEXER PGP comma\n");
		return COMMA;
	}
{QSTRING}		{
		SIPDEBUG("SIP PARSER LEXER PGP quoted string \n");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{TOKEN}		{
		SIPDEBUG("SIP PARSER LEXER PGP Token \n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{TOKENEQUALS}		{
		SIPDEBUG("SIP PARSER LEXER PGP tokenequals \n");
		PASSTOKEN(yytext)
		return TOKENEQUALS;
	}
{COMMATOKENEQUALS}	{
		SIPDEBUG("SIP PARSER LEXER PGP commatokenequals \n");
		PASSTOKEN(yytext)
		return COMMATOKENEQUALS;
	}
.|\n {
		SIPDEBUG("SIP PARSER LEXER PGP default rule \n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}
void sip_lex_Pgp_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Pgp_reset_state(void)
{
	BEGIN 0;
}

int sip_lex_Pgp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexPgpMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserPgpJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Pgp_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexPgpMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Pgp_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexPgpMutex);
#endif
}

int glbSipParserPgplex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserPgplexreal(yylval));
#else
	if (setjmp(glbSipParserPgpJumpBuf) == 0)
	{
		return(glbSipParserPgplexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexPgpMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Pgp_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserPgpJumpBuf,1);
}
#endif

