%{
 /***********************************************************************
 ** FUNCTION:
 **             Lex file for Contact  headers

 *********************************************************************
 **
 ** FILENAME:
 ** Contact.l
 **
 ** DESCRIPTION:
 **
 **
 ** DATE        NAME         		REFERENCE          REASON
 ** ----		----	     		---------	       --------
 ** 24/11/99  KSBinu, Arjun RC			       		  Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "siplist.h"
#include "Contact.tab.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Contact_fatal_error(msg)
static jmp_buf glbSipParserContactJumpBuf;
void sip_lex_Contact_fatal_error(const SIP_S8bit *pErrMsg);
#endif

#define yylval glbSipParserContactlval
int glbSipParserAllowCparams;
int glbSipParserReplacesStart=0;
int glbSipParserContactStart=0;
int glbSipParserAllowSipUrlParams;
int glbSipParserContactAllowSipColon;
#define YY_DECL int glbSipParserContactlexreal(YYSTYPE *lval)
%}
%s AllowQString
%s AllowQString1
%s AllowUserinfo
GMT [Gg][Mm][Tt]
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

STAR "*"
SLASH "/"
SEMICOLON ";"
EQUALS "="
LWS [ \t]*
COLON ":"
COMMA ","
DOUBLEQUOTES "\""
AMPERSAND "&"

/* NEXTONE - added "#" */
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"|"#")

ESCAPED "%"{HEX}{HEX}
UNRESERVED ({MARK}|{ALPHANUM}|"?"|"/")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
WEEKDAY "Sun"|"Mon"|"Tue"|"Wed"|"Thu"|"Fri"|"Sat"
MONTH "Jan"|"Feb"|"Mar"|"Apr"|"May"|"Jun"|"Jul"|"Aug"|"Sep"|"Oct"|"Nov"|"Dec"
RANGLE ">"
AT "@"
OPENBRACE  "{"
CLOSEBRACE "}"
BACKSLASH  "\\"
NUMERIC_RELATION ("<="|">=")
HASH "#"
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"
SEMTOKEN ";"{LWS}{TOKEN}
SEMPARAM ("&"|"$"|"["|"]"|":"|"/"|"("|")")+
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
QSTRING1 ("%22"(([^"\\\n])|("\\".))*"%22")
QSTRING "\""(([^"\\\n])|("\\".))*["]
TEXTUTF8NOCOLON ([\x20-\x39]|[\x3b-\x7f]|" "|"\t")+
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
Q ";"{LWS}[Qq]
EXPIRES ";"{LWS}[Ee][Xx][Pp][Ii][Rr][Ee][Ss]
QUESTION "?"
CONTACTF ([Cc][Oo][Nn][Tt][Aa][Cc][Tt]){LWS}":"
CONTACTS ([Mm]){LWS}":" 
DISPNAME ((({TOKEN}|{LWS})*)|({LWS}{QSTRING}{LWS}))"<"
CALLIDF ([Cc][Aa][lL][lL]-[Ii][Dd]){LWS}":"
CALLIDS [Ii]{LWS}":"
REPLACES ([Rr][Ee][Pp][Ll][Aa][Cc][Ee][Ss]){LWS}":"
REPLYTO ([Rr][Ee][Pp][Ll][Yy][-][Tt][Oo]){LWS}":"
FROMTAG ([;]){LWS}([f][r][o][m][-][t][a][g])
TOTAG 	([;]){LWS}([t][o][-][t][a][g])
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
ALSOF [Aa][Ll][Ss][Oo]{LWS}":"
JOIN ([Jj][Oo][Ii][Nn]){LWS}":"
%%
{LWS} ;
{FROMTAG}	{
		if(!glbSipParserReplacesStart)REJECT;
		BEGIN AllowQString1;
		SIPDEBUG("SIP PARSER LEXER REPLACES fromtag\n");
		return FROMTAG;
		}
{TOTAG}		{
		if(!glbSipParserReplacesStart)REJECT;
		BEGIN AllowQString1;
		SIPDEBUG("SIP PARSER LEXER REPLACES totag\n");
		return TOTAG;
		}
"<" {
		SIPDEBUG("SIP PARSER LEXER CONTACT Langle\n");
		PASSTOKEN(yytext)
		if(glbSipParserContactStart)
			glbSipParserAllowCparams = 1;
		glbSipParserAllowSipUrlParams = 0;
		BEGIN 0 ;
		return LANGLE ;
		}
{REPLACES}	{
		BEGIN 0;
		glbSipParserAllowCparams=0;
		glbSipParserContactStart = 0;
		glbSipParserReplacesStart = 1;
		SIPDEBUG("SIP PARSER LEXER REPLACES replaces\n");
		return REPLACES;
		}
{REPLYTO}	{
		BEGIN 0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 0;
		glbSipParserAllowCparams=0;
		SIPDEBUG("SIP PARSER LEXER REPLACES replyto\n");
		return REPLYTO;
		}
{CALLIDF} {
		BEGIN 0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 0;
		glbSipParserContactAllowSipColon =1;
		SIPDEBUG("SIP PARSER LEXER CONTACT callidf\n"); 
		PASSCHAR('f')
		return CALLID;
	}
{CALLIDS} {
		BEGIN 0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 0;
		glbSipParserContactAllowSipColon =1;
		SIPDEBUG("SIP PARSER LEXER CONTACT callids\n"); 
		PASSCHAR('s')
		return CALLID;
	}
{GMT}		{ 
		SIPDEBUG("SIP PARSER LEXER CONTACT gmt\n");
		return GMT;
	}
{ALSOF}			{
		BEGIN 0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 0;
		SIPDEBUG("SIP PARSER LEXER  CONTACT alsof\n");
		PASSCHAR('f')
		return ALSO;
	}
{CONTACTF}	{ 
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 1;
		SIPDEBUG("SIP PARSER LEXER CONTACT contactf \n"); 
		PASSCHAR('f')
		return CONTACT; 
	}
{CONTACTS}	{ 
		BEGIN 0;
		glbSipParserAllowCparams=1;
		glbSipParserAllowSipUrlParams=0;
		glbSipParserReplacesStart = 0;
		glbSipParserContactStart = 1;
		SIPDEBUG("SIP PARSER LEXER CONTACT fixed\n"); 
		PASSCHAR('s')
		return CONTACT; 
	}
{Q} 		{ 
		if(!glbSipParserAllowCparams) REJECT; 
		SIPDEBUG("SIP PARSER LEXER CONTACT ;q\n"); 
		return QVAL; 
	}
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER CONTACT sip:\n");
		if (!glbSipParserContactAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER CONTACT starting state userinfoat\n");
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
		SIPDEBUG("SIP PARSER LEXER CONTACT sips:\n");
		if (!glbSipParserContactAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER CONTACT starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPSCOLON;
		}
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}

{EXPIRES}	{ 
		if(!glbSipParserAllowCparams) REJECT;
		SIPDEBUG("SIP PARSER LEXER CONTACT expires\n");
		return EXPIRES;
	}
<AllowQString,AllowQString1>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT qstring\n");
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
{STAR}	{
	  	SIPDEBUG("SIP PARSER LEXER CONTACT star\n");
		PASSTOKEN(yytext)
   		return STAR;
	}
{COLON}		{
	  	SIPDEBUG("SIP PARSER LEXER CONTACT colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT semicolon\n");
		PASSTOKEN(yytext)
   		return SEMICOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{AMPERSAND}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT ampersand\n");
		PASSTOKEN(yytext)
   		return AMPERSAND;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT comma\n");
		PASSTOKEN(yytext)
   		return COMMA;
	}
{DOUBLEQUOTES}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT doublequotes\n");
		PASSTOKEN(yytext)
   		return DOUBLEQUOTES;
	}
{OPENBRACE}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT open brace\n");
		PASSTOKEN(yytext)
   		return OPENBRACE;
	}
{CLOSEBRACE}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT close brace\n");
		PASSTOKEN(yytext)
   		return CLOSEBRACE;
	}
{BACKSLASH}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT back slash\n");
		PASSTOKEN(yytext)
   		return BACKSLASH;
	}
{HASH} {
		SIPDEBUG("SIP PARSER LEXER CONTACT hash\n");
		PASSTOKEN(yytext)
		return HASH;
  }
{NUMERIC_RELATION} {
		SIPDEBUG("SIP PARSER LEXER CONTACT numeric_relation\n");
		PASSTOKEN(yytext)
		return NUMERIC_RELATION;
  }
{QUESTION}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT right angle\n");
		PASSTOKEN(yytext)
		if(glbSipParserContactStart)
			glbSipParserAllowCparams = 1;
		glbSipParserAllowSipUrlParams = 0;
		BEGIN 0 ;
   		return RANGLE;
	}
{AT}		{
		SIPDEBUG("SIP PARSER LEXER CONTACT at\n");
		PASSTOKEN(yytext)
   		return AT;
	}
{DIGIT2}	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT digit2\n");
		return DIGIT2;
	}
{DIGIT4} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT digit4\n");
		return DIGIT4 ;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT digits\n");
		return DIGITS ;
	}
{SIPFLOAT} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT float\n");
		return SIPFLOAT ;
	}
{MONTH} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT Month\n");
		return MONTH ;
	}
{WEEKDAY} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER CONTACT weekday\n");
		return WEEKDAY ;
	}
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER CONTACT dispname\n");
		PASSTOKEN(yytext)
		glbSipParserAllowCparams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
<AllowUserinfo>{USERINFOAT} 	{
	 	SIPDEBUG("SIP PARSER LEXER CONTACT userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER CONTACT semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMTOKEN;
	}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER CONTACT semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{TOKEN} 	{
	  	SIPDEBUG("SIP PARSER LEXER CONTACT Token \n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return TOKEN;
	}
<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER Contact semparamvalue\n");
		PASSTOKEN(yytext);
		BEGIN 0;
		return SEMPARAMVALUE;
	}
{IPV6ADDR}	{
		SIPDEBUG("SIP PARSER LEXER CONTACT ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER CONTACT uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}
.|\n		{
		  SIPDEBUG("SIP PARSER LEXER CONTACT default rule\n");
			return DEFAULT;
		}

{JOIN}	{
		BEGIN 0;
		glbSipParserAllowCparams=0;
		glbSipParserContactStart = 0;
		glbSipParserReplacesStart = 1;
		SIPDEBUG("SIP PARSER LEXER JOIN join\n");
		return JOIN;
		}
%%



int yywrap(void)
{
 return 1;
}


void sip_lex_Contact_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Contact_reset_state(void)
{
	BEGIN 0;
	glbSipParserContactAllowSipColon=0;
}

int sip_lex_Contact_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexContactMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserContactJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Contact_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexContactMutex);
#endif
		return 1;
	}
#endif	
}

void sip_lex_Contact_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexContactMutex);
#endif
}

int glbSipParserContactlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserContactlexreal(yylval));
#else
	if (setjmp(glbSipParserContactJumpBuf) == 0)
	{
		return(glbSipParserContactlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexContactMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Contact_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserContactJumpBuf,1);
}
#endif
