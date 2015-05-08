%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Date/Time headers

 *********************************************************************
 **
 ** FILENAME:
 ** datetime.l
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
#include "Datetime.tab.h" 
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Datetime_fatal_error(msg)
static jmp_buf glbSipParserDatetimeJumpBuf;
void sip_lex_Datetime_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
#define yylval glbSipParserDatetimelval
void datecommentparse(char *pText, YYSTYPE *lval);
#define YY_DECL int glbSipParserDatetimelexreal(YYSTYPE *lval)
%}
DIGIT [0-9]
DIGIT2 {DIGIT}{2,2}
DIGIT4 {DIGIT}{4,4}
DIGITS {DIGIT}+
SIPFLOAT ({DIGIT}*)"."({DIGIT}*)
LWS [ \t]*
TEXTUTF8 [\x20-\x7f]|" "|"\t"
CHAR [\x00-\x7f]
CTEXT [\x20-\x26]|[\x29-\x5b]|[\x5d-\x7f]|"\t"
QTDPAIR "\\"{CHAR}
COMMENT "("((.|[ \t])*)")"
SEMICOLON ";"
COMMA ","
COLON ":"
EQUALS "="
GMT [Gg][Mm][Tt]
QSTRING "\""(([^"\\\n])|("\\".))*["]
WEEKDAY ([Ss][Uu][Nn])|([Mm][Oo][Nn])|([Tt][Uu][Ee])|([Ww][Ee][Dd])|([Tt][Hh][Uu])|([Ff][Rr][Ii])|([Ss][Aa][Tt])
MONTH ([Jj][Aa][Nn])|([Ff][Ee][Bb])|([Mm][Aa][Rr])|([Aa][Pp][Rr])|([Mm][Aa][Yy])|([Jj][Uu][Nn])|([Jj][Uu][Ll])|([Aa][Uu][Gg])|([Ss][Ee][Pp])|([Oo][Cc][Tt])|([Nn][Oo][Vv])|([Dd][Ee][Cc])
DATE [Dd][Aa][Tt][Ee]{LWS}":" 
EXPIRES [Ee][Xx][Pp][Ii][Rr][Ee][Ss]{LWS}":"
MINSE [Mm][Ii][Nn]-[Ss][Ee]{LWS}":"
TIMESTAMP [Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp]{LWS}":"
RETRYAFTER [Rr][Ee][Tt][Rr][Yy]-[Aa][Ff][Tt][Ee][Rr]{LWS}":"
MINEXPIRES [Mm][Ii][Nn]-[Ee][Xx][Pp][Ii][Rr][Ee][Ss]{LWS}":"
SESSIONEXPIRESF ([Ss][Ee][Ss][Ss][Ii][Oo][Nn]-[Ee][Xx][Pp][Ii][Rr][Ee][Ss]){LWS}":"
SESSIONEXPIRESS ([Xx]){LWS}":" 
SUBSCRIPTIONEXPIRES ([Ss][Uu][Bb][Ss][Cc][Rr][Ii][Pp][Tt][Ii][Oo][Nn]-[Ss][Tt][Aa][Tt][Ee]){LWS}":"
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
%%
{LWS} ;
{EXPIRES}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME expires\n");
		return EXPIRES;
	}
{SUBSCRIPTIONEXPIRES}	{
		SIPDEBUG("SIP PARSER LEXER DATETIME subscriptionexpires\n");
		return SUBSCRIPTIONEXPIRES;
	}
{MINEXPIRES}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME minexpires\n");
		return MINEXPIRES;
	}
{MINSE}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME minse\n");
		return MINSE;
	}
{SESSIONEXPIRESS} {
		SIPDEBUG("SIP PARSER LEXER DATETIME sessionexpires\n");
		PASSCHAR('s')
		return SESSIONEXPIRES;
	 }
{SESSIONEXPIRESF} {
		SIPDEBUG("SIP PARSER LEXER SESSION EXPIRES \n");
		PASSCHAR('f')
		return SESSIONEXPIRES;
	}
{DATE}			{
		SIPDEBUG("SIP PARSER LEXER DATETIME Date\n");
		return DATEHDRNAME;
	}
{TIMESTAMP}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME timestamp\n");
		return TIMESTAMP;
	}
{RETRYAFTER}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME retryafter\n");
		return RETRYAFTER;
	}
{DIGIT2}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME digit2\n");
		PASSTOKEN(yytext)
		return DIGIT2;
	}
{DIGIT4}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME digit4\n");
		PASSTOKEN(yytext)
		return DIGIT4;
	}
{DIGITS}	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER DATETIME digits\n");
		return DIGITS;
	}
{SIPFLOAT} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER DATETIME float\n");
		return SIPFLOAT;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER DATETIME equals\n");
   		return EQUALS;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME colon\n");
   		return COLON;
	}
{SEMICOLON}	{
	 	SIPDEBUG("SIP PARSER LEXER DATETIME semicolon\n");
   		return SEMICOLON;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME comma\n");
   		return COMMA;
	}
{GMT}		{ 
	 	SIPDEBUG("SIP PARSER LEXER DATETIME GMT\n");
		return GMT;
	}
{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER DATETIME QSTRING");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER REQLINE ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{MONTH}		{
		SIPDEBUG("SIP PARSER LEXER DATETIME Month\n");
		PASSTOKEN(yytext)
		return MONTH;
	}
{WEEKDAY}	{
		SIPDEBUG("SIP PARSER LEXER DATETIME weekday\n");
		PASSTOKEN(yytext)
		return WEEKDAY;
	}
{COMMENT}	{
		 SIPDEBUG("SIP PARSER LEXER DATETIME Comment\n");
		 datecommentparse(yytext, lval);
		 return COMMENT;
		}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER DATETIME Token\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER DATETIME Default rule \n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}
/* Special parsing required for comments. Lex has done a greedy search
 and now we need to put back tokens to undo 
 */
void datecommentparse(char* myvar, YYSTYPE *lval)
{
	char* tmptext;
	int bracketcount = 1 ; /* myvar begins with a ( */
	int origlen;
	int ndx = 1;
	int upndx;


	tmptext = myvar;
	origlen = strlen(myvar);
	while ((bracketcount !=0) && (tmptext[ndx]!='\0'))
	{
		if (tmptext[ndx]=='(') bracketcount++;
		else  if (tmptext[ndx]==')') bracketcount--;
		ndx++;
	}
	/* now putback all the extra struff lex picked up */
	upndx=origlen - ndx ;
	while (upndx>0) 
	{
		unput(tmptext[origlen-((origlen-ndx)-upndx)-1]);
		upndx--;
	}

	lval->pToken = myvar;
	lval->dLength = ndx;

}



void sip_lex_Datetime_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Datetime_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexDatetimeMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserDatetimeJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Datetime_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexDatetimeMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Datetime_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexDatetimeMutex);
#endif

}

int glbSipParserDatetimelex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserDatetimelexreal(yylval));
#else
	if (setjmp(glbSipParserDatetimeJumpBuf) == 0)
	{
		return(glbSipParserDatetimelexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexDatetimeMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Datetime_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserDatetimeJumpBuf,1);
}
#endif

