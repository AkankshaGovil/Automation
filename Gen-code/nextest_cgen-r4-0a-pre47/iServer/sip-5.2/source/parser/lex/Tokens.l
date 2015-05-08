%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Token headers

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
 ** 24/11/99  KSBinu, Arjun RC			        Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Tokens.tab.h"
#define yylval glbSipParserTokenslval
#include "siplist.h"
#define YY_DECL int glbSipParserTokenslexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Tokens_fatal_error(msg)
static jmp_buf glbSipParserTokensJumpBuf;
void sip_lex_Tokens_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
ZEROORONE [01]
DIGIT [0-9]
DIGITS {DIGIT}+
HEX ([A-Fa-f]|{DIGIT})
ESCAPED	"%"{HEX}{HEX}
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|",")
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ALPHANUM ({ALPHA}|{DIGIT})
UNRESERVED ({ALPHANUM}|{MARK})
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
LWS [ \t]*
COLON ":"
SEMICOLON ";"
EQUALS "="
QSTRING "\""(([^"\\\n])|("\\".))*["]
COMMA ","
ACCEPTENCODING ([aA][Cc][cC][eE][pP][tT]-[eE][nN][cC][oO][dD][Ii][Nn][Gg]{LWS}":")
CSEQ [Cc][Ss][Ee][Qq]{LWS}":"
REQUESTDISPOSITIONF ([Rr][Ee][Qq][Uu][Ee][Ss][Tt]-[Dd][Ii][Ss][Pp][Oo][Ss][Ii][Tt][Ii][Oo][Nn]){LWS}":"
REQUESTDISPOSITIONS ([Dd]){LWS}":"
CONTENTENCODINGF ([Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Ee][Nn][Cc][Oo][Dd][Ii][Nn][Gg]){LWS}":"
CONTENTENCODINGS ([Ee]){LWS}":"
CONTENTLENGTHF ([Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Ll][Ee][Nn][Gg][Tt][Hh]){LWS}":"
CONTENTLENGTHS ([Ll]){LWS}":"
MAXFORWARDS [Mm][Aa][Xx][-][Ff][Oo][Rr][Ww][Aa][Rr][Dd][Ss]{LWS}":"
PROXYREQUIRE [Pp][Rr][Oo][Xx][Yy]-[Rr][Ee][Qq][Uu][Ii][Rr][Ee]{LWS}":"
REQUIRE [Rr][Ee][Qq][Uu][Ii][Rr][Ee]{LWS}":"
ALLOW [Aa][Ll][Ll][Oo][Ww]{LWS}":"
ALLOWEVENTSF [Aa][Ll][Ll][Oo][Ww]-[Ee][Vv][Ee][Nn][Tt][Ss]{LWS}":"
ALLOWEVENTSS [Uu]{LWS}":"
UNSUPPORTED [Uu][Nn][Ss][Uu][Pp][Pp][Oo][Rr][Tt][Ee][Dd]{LWS}":"
SUPPORTEDF ([Ss][Uu][Pp][Pp][Oo][Rr][Tt][Ee][Dd]{LWS}":") 
SUPPORTEDS ([Kk]{LWS}":") 
HIDE [Hh][Ii][Dd][Ee]{LWS}":"
PRIORITY [Pp][Rr][Ii][Oo][Rr][Ii][Tt][Yy]{LWS}":"
EMERGENCY [Ee][Mm][Ee][Rr][Gg][Ee][Nn][Cc][Yy] 
URGENT [Uu][Rr][Gg][Ee][Nn][Tt] 
NORMAL [Nn][Oo][Rr][Mm][Aa][Ll] 
NONURGENT [Nn][Oo][Nn]-[Uu][Rr][Gg][Ee][Nn][Tt] 
ROUTE [Rr][Oo][Uu][Tt][Ee]
HOP [Hh][Oo][Pp] 
HEX2 [0-9]|[A-F]|[a-f]
HEX4 {HEX2}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
PRIVACY ([Pp][Rr][Ii][Vv][Aa][Cc][Yy]{LWS}":")

QVALUE ({DIGIT}*)"."({DIGIT}*)
%%
{LWS} ;
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN equals\n");
   		return EQUALS;
	}
{SUPPORTEDF} {
		SIPDEBUG("SIP PARSER LEXER TOKENS request supported\n");
		PASSCHAR('f')
		return SUPPORTED;
	}
{SUPPORTEDS} {
		SIPDEBUG("SIP PARSER LEXER TOKENS request supported\n");
		PASSCHAR('s')
		return SUPPORTED;
	}
{HIDE} {
	SIPDEBUG("SIP PARSER LEXER TOKENS hide\n");
	return HIDE;
 }
{PRIORITY} {
		SIPDEBUG("SIP PARSER LEXER TOKENS priority\n");
		return PRIORITY;
	}
{REQUESTDISPOSITIONF} {
		SIPDEBUG("SIP PARSER LEXER TOKENS request disposition\n");
		PASSCHAR('f')
		return REQUESTDISPOSITION;
			}
{REQUESTDISPOSITIONS} {
		SIPDEBUG("SIP PARSER LEXER TOKENS request disposition\n");
		PASSCHAR('s')
		return REQUESTDISPOSITION;
			}
{CSEQ} 	{
		SIPDEBUG("SIP PARSER LEXER TOKENS dCseq\n");
		return CSEQ;
	}
{ACCEPTENCODING}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS acceptencoding\n");
		return ACCEPTENCODING;
	}
{CONTENTENCODINGS}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS contentencoding\n");
		PASSCHAR('s')
		return CONTENTENCODING;
	}
{CONTENTENCODINGF}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS contentencoding\n");
		PASSCHAR('f')
		return CONTENTENCODING;
	}
{CONTENTLENGTHS}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS contentlength\n");
		PASSCHAR('s')
		return CONTENTLENGTH;
	}
{CONTENTLENGTHF}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS contentlength\n");
		PASSCHAR('f')
		return CONTENTLENGTH;
	}
{MAXFORWARDS}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS maxforwards\n");
		return MAXFORWARDS;
	}
{PROXYREQUIRE}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS proxyrequire\n");
		return PROXYREQUIRE;
	}
{REQUIRE}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS require\n");
		return REQUIRE;
	}
{ALLOW}			{
		SIPDEBUG("SIP PARSER LEXER TOKENS allow\n");
		return ALLOW;
	}
{ALLOWEVENTSF}			{
		SIPDEBUG("SIP PARSER LEXER TOKENS allowevents full\n");
		PASSCHAR('f');
		return ALLOWEVENTS;
	}
{ALLOWEVENTSS}			{
		SIPDEBUG("SIP PARSER LEXER TOKENS allowevents short \n");
		PASSCHAR('s');
		return ALLOWEVENTS;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN ipv6addr\n");
		PASSTOKEN(yytext)
		return IPV6ADDR ;
	}
{UNSUPPORTED}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS unsupported\n");
		return UNSUPPORTED;
	}
{ZEROORONE}	 {
		SIPDEBUG("SIP PARSER LEXER TOKENS zerorone\n");
		PASSTOKEN(yytext)
		return ZEROORONE;
	}
{DIGITS}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS digits\n");
		PASSTOKEN(yytext)
		return DIGITS;
	}
{QVALUE}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS qvalues\n");
		PASSTOKEN(yytext)
		return QVALUE;
	}
{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN quoted string\n");
		PASSTOKEN(yytext)
		return QSTRING;
	}	
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER TOKENS pToken\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS colon\n");
   		return COLON;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTENCODING semicolon\n");
   		return SEMICOLON;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER TOKENS comma\n");
   		return COMMA;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER TOKENS default\n");
		return DEFAULT;
	}
{PRIVACY}	{
		SIPDEBUG("SIP PARSER LEXER TOKENS Privacy\n");
		return PRIVACY;
		}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_Tokens_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Tokens_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexTokensMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserTokensJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Tokens_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexTokensMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Tokens_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokensMutex);
#endif
}

int glbSipParserTokenslex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserTokenslexreal(yylval));
#else
	if (setjmp(glbSipParserTokensJumpBuf) == 0)
	{
		return(glbSipParserTokenslexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokensMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Tokens_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserTokensJumpBuf,1);
}
#endif

