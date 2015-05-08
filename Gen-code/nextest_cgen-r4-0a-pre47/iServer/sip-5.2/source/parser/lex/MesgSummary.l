%{
/**********************************************************************
 ** FUNCTION:
 **             Lex file for Message Waiting Summary Msg Body

 *********************************************************************
 **
 ** FILENAME:
 ** MesgSummary.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        	NAME         		REFERENCE          REASON
 ** ----			----	     		---------	       --------
 ** 07/01/2002   Sasidhar P V K			    			 Initial Creation
 **
 **
 **     Copyright 2002, Hughes Software Systems, Ltd. 
 *********************************************************************/

#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "MesgSummary.tab.h"
#define yylval glbSipParserMesgSummarylval
#include "siplist.h"
#define YY_DECL int glbSipParserMesgSummarylexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_MesgSummary_fatal_error(msg)
static jmp_buf glbSipParserMesgSummaryJumpBuf;
void sip_lex_MesgSummary_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
/* This is Used to Reject Media Token Once we get HNAME,HVALUE */
SIP_U8bit neglectMedia = 0;
int glbSipParserMesgSummaryAllowSipColon;
extern int glbSipParserAllowSipUrlParams;
%}
%s AllowQString
%s AllowQString1
%s AllowUserinfo

MWAITING [Mm][Ee][Ss][Ss][Aa][Gg][Ee][Ss][-][Ww][Aa][Ii][Tt][Ii][Nn][Gg]{LWS}":"
MACCOUNT [Mm][Ee][Ss][Ss][Aa][Gg][Ee][-][Aa][Cc][Cc][Oo][Uu][Nn][Tt]{LWS}":"
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})
HNAME ({ALPHANUM}|"_"|"-")+
SP [ ]
LWS [ \t]*
YES [Yy][Ee][Ss]
NO [Nn][Oo]
MEDIATYPE ([Vv]"oicemail"|[Ee]"mail"|[Ff]"ax"|[Vv]"ideo"){LWS}":"  
HVALUE {COLON}{TEXT-UTF8} 
COLON ":"{LWS}
CRLF ("\n"|"\r")("\n"|"\r"|{LWS})*
SLASH "/"
LANGLE "<"
RANGLE ">"
LEFTPARAN "("
RIGHTPARAN ")"
COMMA ","
EQUALS "="
SEMICOLON ";"
TEXT-UTF8  ({TEXT-UTF8char}|{LWS})*
TEXT-UTF8char ([\x20-\x7e])|{UTF8-NONASCII}
UTF8-NONASCII  ([\xc0-\xdf]{UTF8-CONT})|([\xe0-\xef]{UTF8-CONT}{UTF8-CONT})|([\xf0-\xf7]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})|([\xf8-\xfb]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})|([\xfc-\xfd]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})
UTF8-CONT  [\x80-\xbf]

QSTRING "\""(([^"\\\n])|("\\".))*["]
QSTRING1 ("%22"(([^"\\\n])|("\\".))*"%22")
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
UNRESERVED ({MARK}|{ALPHANUM}|"/"|"?")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SEMPARAM ("&"|"$"|"["|"]"|":"|"/"|"("|")")+
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
SEMTOKEN ";"{LWS}{TOKEN}
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"
TAG ";"{LWS}[tT][Aa][Gg]
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"

%%
{MWAITING}  {
  	         SIPDEBUG("SIP PARSER LEXER MesgSummary MWAITING\n");
             PASSTOKEN(yytext);return MWAITING;
            }

{MACCOUNT} {
  	         SIPDEBUG("SIP PARSER LEXER MesgSummary MACCOUNT\n");
             PASSTOKEN(yytext) ;
			 return MACCOUNT;
           }

{COMMA}		{
  	SIPDEBUG("SIP PARSER LEXER MesgSummary comma\n");
		PASSTOKEN(yytext)
		BEGIN 0;
 		return COMMA;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER MesgSummary equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP_PARSER LEXER MesgSummary semicolon");
		PASSTOKEN(yytext)
		return SEMICOLON;
	}
{COLON}	{
		SIPDEBUG("SIP_PARSER LEXER MesgSummary colon");
		PASSTOKEN(yytext)
		return COLON;
	}
{LANGLE} {
		SIPDEBUG("SIP_PARSER LEXER MesgSummary LANGLE");
	  glbSipParserAllowSipUrlParams = 1;
   	return LANGLE;
}
{RANGLE} {
		SIPDEBUG("SIP_PARSER LEXER MesgSummary RANGLE");
	  glbSipParserAllowSipUrlParams = 0;
   	return RANGLE;
}

<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER TO userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
{TAG}	{ 
		SIPDEBUG("SIP PARSER LEXER MesgSummary Tag\n");
		PASSTOKEN(yytext)
		return TAG;
	   }
{LEFTPARAN} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary LEFTPARAN\n");
    PASSTOKEN(yytext);return LEFTPARAN;}

{SLASH} { 
		SIPDEBUG("SIP PARSER LEXER MesgSummary SLASH\n");
        PASSTOKEN(yytext);return SLASH;}

{RIGHTPARAN} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary RIGHTPARAN\n");
    PASSTOKEN(yytext);return RIGHTPARAN; }

{DIGITS} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary DIGITS\n");
    PASSTOKEN(yytext);return DIGITS; }

{YES} { 
		SIPDEBUG("SIP PARSER LEXER MesgSummary YES\n");
    PASSTOKEN(yytext);return YES;}

{NO} { 
		SIPDEBUG("SIP PARSER LEXER MesgSummary NO\n");
    PASSTOKEN(yytext);return NO;}

{MEDIATYPE} { 
		SIPDEBUG("SIP PARSER LEXER MesgSummary MEDIATYPE\n");
    if( neglectMedia )REJECT;PASSTOKEN(yytext);return MEDIATYPE;}

{TOKEN} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary TOKEN\n");
    PASSTOKEN(yytext); 
          return TOKEN;}

{HNAME} { 
		SIPDEBUG("SIP PARSER LEXER MesgSummary HNAME\n");
    PASSTOKEN(yytext);return HNAME;}

{HVALUE} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary HVALUE\n");
    PASSTOKEN(yytext);return HVALUE;}

{CRLF} {
		SIPDEBUG("SIP PARSER LEXER MesgSummary CRLF\n");
    PASSTOKEN(yytext);return CRLF;}

{LWS} ;

{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER MesgSummary SipColon\n");
		if (!glbSipParserMesgSummaryAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER MesgSummary starting state userinfoat\n");
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
		if (!glbSipParserMesgSummaryAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER MesgSummary starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPCOLON;
		}
		else
		{
		SIPDEBUG("SIP PARSER LEXER MesgSummary SipsColon\n");
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
}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER MesgSummary ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER MesgSummary semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMTOKEN;
		}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER MesgSummary semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER MesgSummary uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	
<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER TO semparamvalue\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return SEMPARAMVALUE;
	}
.     { return DEFAULT;}

%%
int yywrap(void)
{
        return(1);
}
void sip_lex_MesgSummary_free(void)
{
        yy_delete_buffer(yy_current_buffer);
}

void sip_lex_MesgSummary_reset_state(void)
{
        BEGIN 0;
}

int sip_lex_MesgSummary_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexMesgSummaryMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserMesgSummaryJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_MesgSummary_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexMesgSummaryMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_MesgSummary_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMesgSummaryMutex);
#endif
}

int glbSipParserMesgSummarylex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserMesgSummarylexreal(yylval));
#else
	if (setjmp(glbSipParserMesgSummaryJumpBuf) == 0)
	{
		return(glbSipParserMesgSummarylexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMesgSummaryMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_MesgSummary_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserMesgSummaryJumpBuf,1);
}
#endif
