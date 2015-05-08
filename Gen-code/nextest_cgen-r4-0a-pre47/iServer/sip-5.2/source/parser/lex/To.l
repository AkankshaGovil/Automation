%{
 /***********************************************************************
 ** FUNCTION:
 **             Lex file for Fromto headers

 *********************************************************************
 **
 ** FILENAME:
 ** tokensltoken.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----		----	     		---------	    	   --------
 ** 24/11/99  KSBinu, Arjun RC			         Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "To.tab.h"
#include "siplist.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_To_fatal_error(msg)
static jmp_buf glbSipParserToJumpBuf;
void sip_lex_To_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
int glbSipParserToAllowSipColon;
int glbSipParserToAllowHeaderParams;
extern int glbSipParserAllowSipUrlParams;
#define YY_DECL int glbSipParserTolexreal(YYSTYPE *lval)
#define yylval glbSipParserTolval
%}
%s AllowQString
%s AllowQString1
%s AllowUserinfo
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
HEX [0-9]|[A-F]|[a-f]
DIGIT [0-9]
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`"|"#")+

EQUALS "="
LWS [ \t]*
SEMICOLON ";"
COLON ":"
COMMA ","

/* NEXTONE - added "#" */
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"|"#")

ESCAPED "%"{HEX}{HEX}
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
UNRESERVED ({MARK}|{ALPHANUM}|"/"|"?")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
WEEKDAY "Sun"|"Mon"|"Tue"|"Wed"|"Thu"|"Fri"|"Sat"
MONTH "Jan"|"Feb"|"Mar"|"Apr"|"May"|"Jun"|"Jul"|"Aug"|"Sep"|"Oct"|"Nov"|"Dec"
LANGLE "<"
RANGLE ">"
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"
TAG ";"{LWS}[tT][Aa][Gg]
REF [rR][Ee][Ff]
REFTOKEN ";"{LWS}{REF}{LWS}"="{LWS}"<"
SCHEME [Ss][Cc][Hh][Ee][Mm][Ee]
SCHEMETOKEN ";"{SCHEME}
SEMPARAM ("&"|"$"|"["|"]"|":"|"/"|"("|")")+
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
SEMTOKEN ";"{LWS}{TOKEN}
QSTRING1 ("%22"(([^"\\\n])|("\\".))*"%22")
QSTRING "\""(([^"\\\n])|("\\".))*["]
TEXTUTF8NOCOLON ([\x20-\x39]|[\x3b-\x7f]|" "|"\t")+
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
QUESTION "?"
DISPNAME ({TOKEN}|{QSTRING}|{LWS})*"<"
FROMF ([Ff][Rr][Oo][Mm]){LWS}":"
FROMS ([Ff]){LWS}":"
TOF ([Tt][Oo]){LWS}":"
TOS ([Tt]){LWS}":"
ROUTE [Rr][Oo][Uu][Tt][Ee]{LWS}":" 
RECORDROUTE [Rr][Ee][Cc][Oo][Rr][Dd]-[Rr][Oo][Uu][Tt][Ee]{LWS}":"
REFERTOS [rR]{LWS}":"
REFERTOF [rR][Ee][Ff][Ee][rR]-[Tt][oO]{LWS}":"
REFERBYS [bB]{LWS}":"
REFERBYF [rR][Ee][Ff][Ee][rR][rR][Ee][Dd]-[bB][yY]{LWS}":"
%%
{LWS} ;
{REFERBYS}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO b:\n");
		PASSCHAR('s')
		return REFERBY;
	}
{REFERBYF}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO Referred-By:\n");
		PASSCHAR('f')
		return REFERBY;
	}
{REFERTOS}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO r:\n");
		PASSCHAR('s')
		return REFERTO;
	}
{REFERTOF}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO refer to:\n");
		PASSCHAR('f')
		return REFERTO;
	}
{FROMS}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO f:\n");
		PASSCHAR('s')
		return FROM;
	}
{FROMF}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO from:\n");
		PASSCHAR('f')
		return FROM;
	}
{TOF}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO To:\n");
		PASSCHAR('f')
		return TO;
	}
{TOS}		{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO T:\n");
		PASSCHAR('s')
		return TO;
	}
{ROUTE}		{ 	
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO Route\n");
		return ROUTE;
	}
{RECORDROUTE}	{ 
		BEGIN 0;
		glbSipParserToAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER TO RecordRoute\n");
		return RECORDROUTE;
	}
{TAG}	{ 
		SIPDEBUG("SIP PARSER LEXER TO Tag\n");
		PASSTOKEN(yytext)
		return TAG;
	}
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER TO sip:\n");
		if (!glbSipParserToAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER TO starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPCOLON;
		}
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}
{SIPSCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER TO sips:\n");
		if (!glbSipParserToAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER TO starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPSCOLON;
		}
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}
<AllowQString,AllowQString1>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER TO qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{QSTRING1}  {
		SIPDEBUG("SIP PARSER LEXER TO qstring1\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING1;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER TO colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{COMMA}		{
	  	SIPDEBUG("SIP PARSER LEXER TO comma\n");
		PASSTOKEN(yytext)
		BEGIN 0;
   		return COMMA;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP_PARSER LEXER FROMTO semicolon");
		PASSTOKEN(yytext)
		return SEMICOLON;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER TO equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER TO question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER TO right angle\n");
		glbSipParserToAllowHeaderParams = 1;
		glbSipParserAllowSipUrlParams = 0;
   		return RANGLE;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER TOfromto digits\n");
		return DIGITS ;
	}
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER TO dispname \n");
		PASSTOKEN(yytext)
		glbSipParserToAllowHeaderParams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER TO userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER FROMTO ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER TO semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMTOKEN;
		}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER TO semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER TO pToken\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER TO uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	

<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER TO semparamvalue\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return SEMPARAMVALUE;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER TO default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_To_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_To_reset_state( void )
{
	BEGIN 0;
	glbSipParserToAllowSipColon=0;
}

int sip_lex_To_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexToMutex, FAST_MXLOCK_EXCLUSIVE );
#endif
#ifdef SIP_NOFLEXRECOVERY
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if (setjmp(glbSipParserToJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_To_scan_buffer "
			"GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexToMutex );
#endif
		return 1;
	}
	
#endif

}
void sip_lex_To_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexToMutex);
#endif

}

int glbSipParserTolex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserTolexreal(yylval));
#else
	if (setjmp(glbSipParserToJumpBuf) == 0)
	{
		return(glbSipParserTolexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexToMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_To_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserToJumpBuf,1);
}
#endif

