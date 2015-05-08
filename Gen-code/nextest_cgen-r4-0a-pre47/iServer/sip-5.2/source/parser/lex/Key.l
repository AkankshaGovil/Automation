%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Key headers

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
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "portlayer.h"
#define yylval glbSipParserKeylval
#include "siplist.h"
#include "Key.tab.h"
#define YY_DECL int glbSipParserKeylexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Key_fatal_error(msg)
static jmp_buf glbSipParserKeyJumpBuf;
void sip_lex_Key_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}

UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
EQUALS "="
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
QSTRING "\""(([^"\\\n])|("\\".))*["]
COMMA ","
LWS [ \t]*
ENCRYPTION [Ee][Nn][Cc][Rr][Yy][Pp][Tt][Ii][Oo][Nn]{LWS}":" 
RESPKEY [Rr][Ee][Ss][Pp][Oo][Nn][Ss][Ee]-[Kk][Ee][Yy]{LWS}":"
CONTENTLANGUAGEVAL ({ALPHA}{1,8})(({LWS}"-"{LWS}{ALPHA}{1,8})*)
CLANGCOLON [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Ll][Aa][Nn][Gg][Uu][Aa][Gg][Ee]{LWS}":"
DIGITS {DIGIT}+
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
SIPFLOAT ({DIGIT}+)"."({DIGIT}+)
%%
{LWS} ;
{ENCRYPTION}	{
		SIPDEBUG("SIP PARSER LEXER KEY encryption\n");
		return ENCRYPTIONCOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER KEY equals\n");
   		return EQUALS;
	}
{CLANGCOLON}	{
		SIPDEBUG("SIP PARSER LEXER KEY ContentLanguage\n");
		return CONTENTLANGUAGE;
	}
{CONTENTLANGUAGEVAL} {
	 	SIPDEBUG("SIP PARSER LEXER KEY ContentLanguageVal\n");
		PASSTOKEN(yytext)
		return CONTENTLANGUAGEVAL;
	}
{SIPFLOAT}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER KEY float\n");
		PASSTOKEN(yytext)
		return SIPFLOAT;	
	}
{IPV6ADDR}  {  
		BEGIN 0;   
		SIPDEBUG("SIP PARSER LEXER KEY ipv6addr\n");
		PASSTOKEN(yytext)
		return IPV6ADDR ;
	}
{RESPKEY}	{
		SIPDEBUG("SIP PARSER LEXER KEY respkey\n");
		return RESPKEY;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER KEY comma\n");
   		return COMMA;
	}
{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER KEY quoted string\n");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER KEY Token\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER KEY default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}


void sip_lex_Key_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Key_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexKeyMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserKeyJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Key_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexKeyMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Key_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexKeyMutex);
#endif
}

int glbSipParserKeylex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserKeylexreal(yylval));
#else
	if (setjmp(glbSipParserKeyJumpBuf) == 0)
	{
		return(glbSipParserKeylexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexKeyMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Key_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserKeyJumpBuf,1);
}
#endif

