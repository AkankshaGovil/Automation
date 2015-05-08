
%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for AcceptContact headers

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
#include "AcceptContact.tab.h"
#include "siplist.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR sip_lex_AcceptContact_fatal_error
static jmp_buf glbSipParserAcceptContactJumpBuf;
void sip_lex_AcceptContact_fatal_error(const SIP_S8bit *pErrMsg);
#endif
#define MIN(a,b) ((a)<(b))?(a):(b)
#define yylval glbSipParserAcceptContactlval
extern int glbSipParserAllowCparams,glbSipParserAllowSipUrlParams;
int glbSipParserAcceptContactAllowSipColon;
#define YY_DECL int glbSipParserAcceptContactlexreal(YYSTYPE *lval)
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

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`"|"#")+

SLASH "/"
SEMICOLON ";"
EQUALS "="
LWS [ \t]*
COLON ":"
COMMA ","
AMPERSAND "&"

/* NEXTONE - added "#" */
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"|"#")

ESCAPED "%"{HEX}{HEX}
UNRESERVED ({MARK}|{ALPHANUM}|"?"|"/")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
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
Q ";"{LWS}[Qq]
QUESTION "?"
CONTACTF ([Cc][Oo][Nn][Tt][Aa][Cc][Tt]){LWS}":"
CONTACTS ([Mm]){LWS}":"
ACCEPTCONTACTF ([Aa][Cc][Cc][Ee][Pp][Tt]-[Cc][Oo][Nn][Tt][Aa][Cc][Tt]){LWS}":"
ACCEPTCONTACTS ([Aa]){LWS}":"
DISPNAME ({TOKEN}|{QSTRING}|{LWS})*"<"
STAR "*"
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
%%
{LWS} ;
{ACCEPTCONTACTF}	{
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT acceptContact\n");
		PASSCHAR('f');
		return ACCEPTCONTACT;
	}
{ACCEPTCONTACTS}	{
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT acceptContact\n");
		PASSCHAR('s');
		return ACCEPTCONTACT;
	}

{Q} 	{
		if(!glbSipParserAllowCparams) REJECT;
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT ;q=\n");
		return QVAL;
	}

{SIPCOLON}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT sip:\n");
		if (!glbSipParserAcceptContactAllowSipColon)
		{
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
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT sips:\n");
		if (!glbSipParserAcceptContactAllowSipColon)
		{
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
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{COLON}		{
	  	SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT semicolon\n");
		PASSTOKEN(yytext)
   		return SEMICOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{AMPERSAND}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT ampersand\n");
		PASSTOKEN(yytext)
   		return AMPERSAND;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT comma\n");
		PASSTOKEN(yytext);
   		return COMMA;
	}
{QUESTION}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT question\n");
		PASSTOKEN(yytext);
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT right angle\n");
		glbSipParserAllowCparams = 1;
		glbSipParserAllowSipUrlParams = 0;
   		return RANGLE;
	}
{AT}		{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT at\n");
		PASSTOKEN(yytext)
   		return AT;
	}
{DIGIT2}	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT digit2\n");
		return DIGIT2;
	}
{DIGIT4} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT digit4\n");
		return DIGIT4 ;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT digits\n");
		return DIGITS ;
	}
{SIPFLOAT} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT float\n");
		return SIPFLOAT ;
	}
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT dispname\n");
		PASSTOKEN(yytext)
		glbSipParserAllowCparams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
<AllowUserinfo>{USERINFOAT} 	{
	 	SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString;
		return SEMTOKEN;
	}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString;
		return SEMPARAMTOKEN;
		}
{STAR}	{
	  	SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT star \n");
		PASSTOKEN(yytext)
		return STAR;
	}
{TOKEN} 	{
	  	SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT Token \n");
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
		SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}
.|\n	{
		  SIPDEBUG("SIP PARSER LEXER ACCEPTCONTACT default rule\n");
			return DEFAULT;
		}
%%
int yywrap()
{
 return 1;
}

void sip_lex_AcceptContact_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_AcceptContact_reset_state(void )
{
	BEGIN 0;
	glbSipParserAcceptContactAllowSipColon=0;
}

int sip_lex_AcceptContact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexAcceptContactMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserAcceptContactJumpBuf) == 0)
	{

		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_AcceptContact_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexAcceptContactMutex);
#endif
		return 1;
	}
#endif
}
void sip_lex_AcceptContact_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexAcceptContactMutex);
#endif
}

int glbSipParserAcceptContactlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserAcceptContactlexreal(yylval));
#else
	if (setjmp(glbSipParserAcceptContactJumpBuf) == 0)
	{
		return(glbSipParserAcceptContactlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexAcceptContactMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_AcceptContact_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy =(SIP_S8bit *) pMessage;
#endif

	sip_error(SIP_Major,pMessage);
	longjmp(glbSipParserAcceptContactJumpBuf,1);
}
#endif



