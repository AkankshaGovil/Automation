
%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for RejectContact headers

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
#include "siplist.h"
#include "RejectContact.tab.h"
#define yylval glbSipParserRejectContactlval
extern int glbSipParserAllowCparams,glbSipParserAllowSipUrlParams;
int glbSipParserRejectContactAllowSipColon;

#define YY_DECL int glbSipParserRejectContactlexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_RejectContact_fatal_error(msg)
static jmp_buf glbSipParserRejectContactJumpBuf;
void sip_lex_RejectContact_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}
%s AllowQString
%s AllowUserinfo
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
HEX [0-9]|[A-F]|[a-f]
DIGIT [0-9]
DIGIT2 {DIGIT}{2,2}
DIGIT4 {DIGIT}{4,4}
DIGITS {DIGIT}+
SIPFLOAT ({DIGIT}*)"."({DIGIT}*)
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
SLASH "/"
SEMICOLON ";"
EQUALS "="
LWS [ \t]*
COLON ":"
COMMA ","
AMPERSAND "&"
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
UNRESERVED ({MARK}|{ALPHANUM}|"?"|"/")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
WEEKDAY "\"Sun"|"\"Mon"|"\"Tue"|"\"Wed"|"\"Thu"|"\"Fri"|"\"Sat"
MONTH "Jan"|"Feb"|"Mar"|"Apr"|"May"|"Jun"|"Jul"|"Aug"|"Sep"|"Oct"|"Nov"|"Dec"
LANGLE "<"
RANGLE ">"
AT "@"
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"
SEMTOKEN ";"{LWS}{TOKEN}
SEMPARAM ("&"|"$"|"["|"]"|":"|"/"|"("|")")+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
QSTRING "\""(([^"\\\n])|("\\".))*["]
TEXTUTF8NOCOLON ([\x20-\x39]|[\x3b-\x7f]|" "|"\t")+
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
COMMENT "("((.|[ \t])*)")"
TCP [tT][cC][pP] 
UDP [uU][dD][pP] 
IP [Ii][Pp] 
PHONE [Pp][Hh][Oo][Nn][Ee] 
Q ";"{LWS}[Qq]
ACTION ";"{LWS}[Aa][Cc][Tt][Ii][Oo][Nn] 
EXPIRES ";"{LWS}[Ee][Xx][Pp][Ii][Rr][Ee][Ss] 
DQUOTE "\""
QUESTION "?"
CONTACTF ([Cc][Oo][Nn][Tt][Aa][Cc][Tt]){LWS}":"
CONTACTS ([Mm]){LWS}":" 
REJECTCONTACTF ([Rr][Ee][Jj][Ee][Cc][Tt]-[Cc][Oo][Nn][Tt][Aa][Cc][Tt]){LWS}":"
REJECTCONTACTS ([Jj]){LWS}":"
GMT [Gg][Mm][Tt][\"]
DISPNAME ({TOKEN}|{QSTRING}|{LWS})*"<"
STAR "*"
%%
{LWS} ;
{REJECTCONTACTF}	{ 
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT RejectContact\n");
		PASSCHAR('f');
		return REJECTCONTACT; 
	}
{REJECTCONTACTS}	{ 
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT RejectContact\n");
		PASSCHAR('s');
		return REJECTCONTACT; 
	}

{Q} 		{ 
		if(!glbSipParserAllowCparams) REJECT; 
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT ;q=\n"); 
		return QVAL; 
	}
	
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT sip:\n");
		if (!glbSipParserRejectContactAllowSipColon)
		{	
			SIPDEBUG\
				("SIP PARSER LEXER REJTCONTACT starting state userinfoat\n");
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
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT sip:\n");
		if (!glbSipParserRejectContactAllowSipColon)
		{	
			SIPDEBUG\
				("SIP PARSER LEXER REJTCONTACT starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPSCOLON;
		}
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}
	
<AllowQString>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{COLON}		{
	  	SIPDEBUG("SIP PARSER LEXER REJECTCONTACT colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT semicolon\n");
		PASSTOKEN(yytext)
   		return SEMICOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{AMPERSAND}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT ampersand\n");
		PASSTOKEN(yytext)
   		return AMPERSAND;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT comma\n");
   		return COMMA;
	}
{QUESTION}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT right angle\n");
		glbSipParserAllowCparams = 1;
		glbSipParserAllowSipUrlParams = 0;
   		return RANGLE;
	}
{AT}		{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT at\n");
		PASSTOKEN(yytext)
   		return AT;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT digits\n");
		return DIGITS ;
	}
{SIPFLOAT} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT float\n");
		return SIPFLOAT ;
	}
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT dispname\n");
		PASSTOKEN(yytext)
		glbSipParserAllowCparams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
<AllowUserinfo>{USERINFOAT} 	{
	 	SIPDEBUG("SIP PARSER LEXER REJECTCONTACT userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString;
		return SEMTOKEN;
	}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString;
		return SEMPARAMTOKEN;
		}	
{STAR}	{
	  	SIPDEBUG("SIP PARSER LEXER REJECTCONTACT star \n");
		PASSTOKEN(yytext)
		return STAR;
	}
{TOKEN} 	{
	  	SIPDEBUG("SIP PARSER LEXER REJECTCONTACT Token \n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}
.|\n	{	
		SIPDEBUG("SIP PARSER LEXER REJECTCONTACT default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}


void sip_lex_RejectContact_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_RejectContact_reset_state(void)
{
	BEGIN 0;
	glbSipParserRejectContactAllowSipColon=0;
}

int sip_lex_RejectContact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexRejectContactMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserRejectContactJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_RejectContact_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexRejectContactMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_RejectContact_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexRejectContactMutex);
#endif

}

int glbSipParserRejectContactlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserRejectContactlexreal(yylval));
#else
	if (setjmp(glbSipParserRejectContactJumpBuf) == 0)
	{
		return(glbSipParserRejectContactlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexRejectContactMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_RejectContact_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserRejectContactJumpBuf,1);
}
#endif

