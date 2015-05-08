
%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Request Line

 *********************************************************************
 **
 ** FILENAME:
 ** tokensltoken.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----     	----	     		---------	       --------
 ** 24/11/99  KSBinu, Arjun RC			         Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Reqline.tab.h"
#define yylval glbSipParserReqlinelval
#include "siplist.h"
#define YY_DECL int glbSipParserReqlinelexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Reqline_fatal_error(msg)
static jmp_buf glbSipParserReqlineJumpBuf;
void sip_lex_Reqline_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}
%s AllowQString
%s AllowQString1
%s AllowUserinfo
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
SIPFLOAT ({DIGIT}+)"."({DIGIT}+)
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"#")+

COLON ":"
AMPERSAND "&"
EQUALS "="
SEMICOLON ";"
COMMA ","
AT "@"
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
ESCAPED "%"{HEX}{HEX}

/* NEXTONE - added "#" */
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"|"#")

UNRESERVED ({MARK}|{ALPHANUM}|"?"|"/")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
SIPSLASH [Ss][Ii][Pp]{LWS}"/"
SIPSLASHFLOAT {SIPSLASH}{LWS}{SIPFLOAT}
LWS [ \t]*
QUESTION "?"
QSTRING1 ("%22"(([^"\\\n])|("\\".))*"%22")
QSTRING "\""(([^"\\\n])|("\\".))*["]
SEMTOKEN ";"{LWS}{TOKEN}
SEMPARAM ("&"|"$"|"["|"]"|":"|"/"|"("|")")+
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
LINEEND "\n"|"\r"|"\r\n"
LINEFOLD ("\n"{LWS})|("\r"{LWS})
%%
{LWS} ;
{SIPSLASHFLOAT}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE sipslashFloat\n");
		
		PASSTOKEN(yytext)
		return SIPSLASHFLOAT;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER REQLINE colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{COMMA} {
		SIPDEBUG("SIP PARSER LEXER REQLINE comma\n");
		PASSTOKEN(yytext)
   		return COMMA;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE semicolon\n");
		PASSTOKEN(yytext)
   		return SEMICOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{AMPERSAND}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE ampersand\n");
		PASSTOKEN(yytext)
   		return AMPERSAND;
	}
{AT}		{
		SIPDEBUG("SIP PARSER LEXER REQLINE at\n");
		PASSTOKEN(yytext)
   		return AT;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER REQLINE digits\n");
		return DIGITS ;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER REQLINE ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{SIPFLOAT} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER REQLINE float\n");
		return SIPFLOAT ;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER FFROMTO question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER FFROMTO sip:\n");
		BEGIN AllowUserinfo;
		return SIPCOLON;
	}
{SIPSCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER FFROMTO sips:\n");
		BEGIN AllowUserinfo;
		return SIPSCOLON;
	}
<AllowQString,AllowQString1>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{QSTRING1}  {
		SIPDEBUG("SIP PARSER LEXER FFROMTO qstring1**\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING1;
	}
<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
<AllowQString1>{SEMPARAMVALUE}     {
        SIPDEBUG("SIP PARSER LEXER REQLINE semparamvalue\n");
        PASSTOKEN(yytext)
		BEGIN 0;
        return SEMPARAMVALUE;
    }
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER REQLINE Token \n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER REQLINE uricnocolon \n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}
{SEMTOKEN} 	{
	 	SIPDEBUG("SIP PARSER LEXER REQLINE SEMTOKEN \n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1	;
		return SEMTOKEN;
	}
{SEMPARAMTOKEN}     {
        SIPDEBUG("SIP PARSER LEXER REQLINE semparamtoken\n");
        PASSTOKEN(yytext)
		BEGIN AllowQString1;
        return SEMPARAMTOKEN;
        }
{LINEEND}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE lineend \n");
		return LINEEND;
	}
{LINEFOLD} {
		SIPDEBUG("SIP PARSER LEXER REQLINE linefold - ignored\n");
 	}
.|\n 	{
		SIPDEBUG("SIP PARSER LEXER REQLINE default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}
void sip_lex_Reqline_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Reqline_reset_state(void)
{
	BEGIN 0;
}

int sip_lex_Reqline_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexReqlineMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserReqlineJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Reqline_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexReqlineMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Reqline_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexReqlineMutex);
#endif
}

int glbSipParserReqlinelex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserReqlinelexreal(yylval));
#else
	if (setjmp(glbSipParserReqlineJumpBuf) == 0)
	{
		return(glbSipParserReqlinelexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexReqlineMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Reqline_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserReqlineJumpBuf,1);
}
#endif

