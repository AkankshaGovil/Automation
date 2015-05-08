%{
 /***********************************************************************
 ** FUNCTION:
 **             Lex file for 3GPP headers(as defined in RFC 3455)

 *********************************************************************
 **
 ** FILENAME:
 ** 3gpp.l
 **
 ** DESCRIPTION:
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----		----	     		---------	    	   --------
 ** 12 Dec,03   S.Chatterjee                            Initial creation
 **
 **     Copyright 2003, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "3gpp.tab.h"
#include "siplist.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_3gpp_fatal_error(msg)
static jmp_buf glbSipParser3gppJumpBuf;
void sip_lex_3gpp_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
int glbSipParser3gppAllowSipColon;
int glbSipParserAllow3gppHeaderParams;
extern int glbSipParserAllowSipUrlParams;
#define YY_DECL int glbSipParser3gpplexreal(YYSTYPE *lval)
#define yylval glbSipParser3gpplval
%}
%s AllowQString1
%s AllowUserinfo
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
HEX [0-9]|[A-F]|[a-f]
DIGIT [0-9]
DIGITS {DIGIT}+
CID	";"{LWS}[Cc][Ii][Dd]
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
EQUALS "="
LWS [ \t]*
SEMICOLON ";"
COLON ":"
COMMA [,]+
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
UNRESERVED ({MARK}|{ALPHANUM}|"/"|"?")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
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
PASSOCIATEDURI [Pp]-[Aa][Ss][Ss][Oo][Cc][Ii][Aa][Tt][Ee][Dd]-[Uu][Rr][Ii]{LWS}":"
PCALLEDPARTYID [Pp]-[Cc][Aa][Ll][Ll][Ee][Dd]-[Pp][Aa][Rr][Tt][Yy]-[Ii][Dd]{LWS}":"
PVISITEDNETWORKID  [Pp]-[Vv][Ii][Ss][Ii][Tt][Ee][Dd]-[Nn][Ee][Tt][Ww][Oo][Rr][Kk]-[Ii][Dd]{LWS}":"
PCFADDR [Pp]-[Cc][Hh][Aa][Rr][Gg][Ii][Nn][Gg]-[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]-[Aa][Dd][Dd][Rr][Ee][Ss][Ss][Ee][Ss]{LWS}":"
%%
{LWS} ;

{TAG}	{ 
		SIPDEBUG("SIP PARSER LEXER 3GPP Tag\n");
		PASSTOKEN(yytext)
		return TAG;
	}
{REFTOKEN}	{
	  	SIPDEBUG("SIP PARSER LEXER 3GPP Ref\n");
		PASSTOKEN(yytext)
		BEGIN 0;
   		return REFTOKEN;
    }
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER 3GPP sip:\n");
		if (!glbSipParser3gppAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER 3GPP starting state userinfoat\n");
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
		SIPDEBUG("SIP PARSER LEXER 3GPP sip:\n");
		if (!glbSipParser3gppAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER 3GPP starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPSCOLON;
		}	
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}

{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER 3GPP qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{QSTRING1}  {
		SIPDEBUG("SIP PARSER LEXER 3GPP qstring1\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING1;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER 3GPP colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{COMMA}		{
		PASSTOKEN(yytext)
		BEGIN 0;
	 	 	SIPDEBUG("SIP PARSER LEXER 3GPP comma\n");
	   		return COMMA;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP_PARSER LEXER 3GPP semicolon");
		PASSTOKEN(yytext)
		return SEMICOLON;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER 3GPP equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER 3GPP question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER 3GPP right angle\n");
		glbSipParserAllow3gppHeaderParams = 1;
		glbSipParserAllowSipUrlParams = 0;
   		return RANGLE;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER 3GPP3gpp digits\n");
		return DIGITS ;
	}
	
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP dispname \n");
		PASSTOKEN(yytext)
		glbSipParserAllow3gppHeaderParams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
		
<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP userinfoat\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return USERINFOAT ;
	}
	
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER 3GPP ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}

{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMTOKEN;
		}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP pToken\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return TOKEN;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	

<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER 3GPP semparamvalue\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return SEMPARAMVALUE;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER 3GPP default rule\n");
		return DEFAULT;
	}
{PASSOCIATEDURI}		{ 	
		BEGIN 0;
		glbSipParserAllow3gppHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER 3GPP PASSOCIATEDURI\n");
		return PASSOCIATEDURI;
	}
{PCALLEDPARTYID}		{ 	
		BEGIN 0;
		glbSipParserAllow3gppHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER 3GPP PCALLEDPARTYID\n");
		return PCALLEDPARTYID;
	}
{PVISITEDNETWORKID}		{ 	
		BEGIN 0;
		glbSipParserAllow3gppHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER 3GPP PVISITEDNETWORKID\n");
		return PVISITEDNETWORKID;
	}
{PCFADDR}		{ 	
		BEGIN 0;
		glbSipParserAllow3gppHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER 3GPP PCFADDR\n");
		return PCFADDR;
	}

%%
int yywrap(void)
{
 return 1;
}

void sip_lex_3gpp_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_3gpp_reset_state( void )
{
	BEGIN 0;
	glbSipParser3gppAllowSipColon=0;
}
int sip_lex_3gpp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLex3gppMutex, FAST_MXLOCK_EXCLUSIVE );
#endif
	BEGIN 0;
#ifdef SIP_NOFLEXRECOVERY
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if (setjmp(glbSipParser3gppJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_3gpp_scan_buffer "
			"GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLex3gppMutex );
#endif
		return 1;
	}
	
#endif

}
void sip_lex_3gpp_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLex3gppMutex);
#endif

}

int glbSipParser3gpplex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParser3gpplexreal(yylval));
#else
	if (setjmp(glbSipParser3gppJumpBuf) == 0)
	{
		return(glbSipParser3gpplexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLex3gppMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_3gpp_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParser3gppJumpBuf,1);
}
#endif

