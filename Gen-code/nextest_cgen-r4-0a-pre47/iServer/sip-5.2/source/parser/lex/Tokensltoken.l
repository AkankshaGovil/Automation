%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Token/Token headers

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
#include "sipdecodeintrnl.h"
#include "Tokensltoken.tab.h"
#include "sipparserinc.h"
#include "siplist.h"
#define yylval glbSipParserTokensltokenlval
#include "siplist.h"
#define YY_DECL int glbSipParserTokensltokenlexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Tokensltoken_fatal_error(msg)
static jmp_buf glbSipParserTokensltokenJumpBuf;
void sip_lex_Tokensltoken_fatal_error(const SIP_S8bit *pErrMsg); 
#endif


%}
%s AllowWord

DIGIT [0-9]
DIGITS {DIGIT}+
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"

UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
SIPFLOAT ({DIGIT}+)"."({DIGIT}+)
DIGIT3 {DIGIT}{3,3}
ALPHANUM ({ALPHA}|{DIGIT})
URI "<"[^>\n]*">" 
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
WORD_LEX ({TOKEN}|"<"|">"|"("|")"|"?"|"["|"]"|"{"|"}"|"\\"|"/"|":"|"\"")+
SLASH "/"
SEMICOLON ";"
EQUALS "="
LWS [ \t]*
COLON ":"
COMMA ","
WARNING [Ww][Aa][Rr][Nn][Ii][Nn][Gg]{LWS}{COLON} 
ACCEPT ([Aa][Cc][Cc][Ee][Pp][Tt]){LWS}{COLON}
ACCEPTLANG [Aa][Cc][Cc][Ee][Pp][Tt][-][Ll][Aa][Nn][Gg][Uu][Aa][Gg][Ee]{LWS}{COLON}
CONTENTTYPEF [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Tt][Yy][Pp][Ee]{LWS}{COLON}
CONTENTTYPES ([Cc]{LWS}{COLON})
EVENTF ([Ee][Vv][Ee][Nn][Tt]{LWS}{COLON})
EVENTS ([Oo]{LWS}{COLON})
QPARAM (";"{LWS}"q"{LWS}"="{LWS}{TOKEN})
QSTRING "\""(([^"\\\n])|("\\".))*["]
LRANGE ({ALPHA}{1,8}({LWS}"-"{LWS}({ALPHA}{1,8}))*)|"*"
LRANGE_INREP ({ALPHANUM}{1,8}({LWS}"-"{LWS}({ALPHANUM}{1,8}))*)|"*"
MIMEVERSION [Mm][Ii][Mm][Ee]-[Vv][Ee][Rr][Ss][Ii][Oo][Nn]{LWS}{COLON} 
ALERTINFO [Aa][Ll][Ee][Rr][Tt]-[Ii][Nn][Ff][Oo]{LWS}{COLON}
CALLIDAT ({WORD_LEX}){LWS}"@"{LWS}({WORD_LEX})
INREPLYTO [Ii][Nn]-[Rr][Ee][Pp][Ll][Yy]-[Tt][Oo]{LWS}{COLON}
CALLINFO [Cc][Aa][Ll][Ll]-[Ii][Nn][Ff][Oo]{LWS}{COLON}
ERRORINFO [Ee][Rr][Rr][Oo][Rr]-[Ii][Nn][Ff][Oo]{LWS}{COLON}
CONTENTDISPOSITION [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Dd][Ii][Ss][Pp][Oo][Ss][Ii][Tt][Ii][Oo][Nn]{LWS}":"
REASON [Rr][Ee][Aa][Ss][Oo][Nn]{LWS}":"
AUTHINFO [Aa][Uu][Tt][Hh][Ee][Nn][Tt][Ii][Cc][Aa][Tt][Ii][Oo][Nn]-[Ii][Nn][Ff][Oo]{LWS}{COLON}
%%
{LWS} ;
{EQUALS}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN equals\n");
   		return EQUALS;
	}

{CONTENTDISPOSITION}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN contentdisposition\n");
		return CONTENTDISPOSITION;
	}
{ERRORINFO}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN errorinfo\n");
		PASSTOKEN(yytext)
		return ERRORINFO;
	}
{REASON}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN reason\n");
		return REASON;
	}
<AllowWord>{LRANGE_INREP}	{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN lrange_inrep\n");
		PASSTOKEN(yytext);
		return LRANGE_INREP;
	}
{LRANGE}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN lrange\n");
		PASSTOKEN(yytext);
		return LRANGE;
	}
{QPARAM}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN qparam\n");
		PASSTOKEN(yytext)
		return QPARAM;
	}
<AllowWord>{CALLIDAT}	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN AT\n");
		return CALLIDAT;
	}
{INREPLYTO}	{
		BEGIN AllowWord;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN inreplyto\n");
		return INREPLYTO;
	}
{ALERTINFO}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN alertinfo\n");
		return ALERTINFO;
	}
{AUTHINFO}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER PGP AuthenticationInfo\n");
		return AUTHINFO;
	}
{CALLINFO}	{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN callinfo\n");
		BEGIN AllowWord;	
		return CALLINFO;
	}
{WARNING}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN warning\n");
		return WARNING;
	}
{MIMEVERSION}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN mimeversion\n");
		return MIMEVERSION;
	}
{ACCEPTLANG}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN acceptlang\n");
		return ACCEPTLANG;
	}
{CONTENTTYPEF}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN contenttype\n");
		PASSCHAR('f')
		return CONTENTTYPE;
	}
{CONTENTTYPES}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN contenttype\n");
		PASSCHAR('s')
		return CONTENTTYPE;
	}
{EVENTF} {
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN event full\n");
		PASSCHAR('f');
		return EVENT;
	}
{EVENTS} {
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN event short \n");
		PASSCHAR('s');
		return EVENT;
	}
{ACCEPT}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN accept\n");
		return ACCEPT;
	}
{QSTRING}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN quoted string\n");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{DIGIT3}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN digit3\n");
		PASSTOKEN(yytext)
		return DIGIT3;	
	}
{SIPFLOAT}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN float\n");
		PASSTOKEN(yytext)
		return SIPFLOAT;	
	}
{TOKEN} 	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN Token\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{URI}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN uri\n");
		PASSTOKEN(yytext)
		return URI;
	}

<AllowWord>{WORD_LEX} 	{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN word\n");
		PASSTOKEN(yytext)
		return WORD_LEX;
	}
{SLASH}		{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN slash\n");
 		return SLASH;
	}
{COLON}		{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN colon\n");
   		return COLON;
	}
{SEMICOLON}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN semicolon\n");
   		return SEMICOLON;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN comma\n");
   		return COMMA;
	}
{IPV6ADDR}  {  
		BEGIN 0;   
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN ipv6addr\n");
		PASSTOKEN(yytext)
		return IPV6ADDR ;
	}
.|\n {
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER TOKENSLASHTOKEN default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}


void sip_lex_Tokensltoken_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Tokensltoken_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexTokensltokenMutex, FAST_MXLOCK_EXCLUSIVE );
#endif
	BEGIN 0;
#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserTokensltokenJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Tokensltoken_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexTokensltokenMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Tokensltoken_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokensltokenMutex);
#endif

}

int glbSipParserTokensltokenlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserTokensltokenlexreal(yylval));
#else
	if (setjmp(glbSipParserTokensltokenJumpBuf) == 0)
	{
		return(glbSipParserTokensltokenlexreal(yylval));
	}
	else
	{
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokensltokenMutex);
#endif
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing");
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Tokensltoken_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserTokensltokenJumpBuf,1);
}
#endif

