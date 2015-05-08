%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Media

 *********************************************************************
 **
 ** FILENAME:
 ** slMedia.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE    	NAME         		REFERENCE           REASON
 ** ----		---	     			---------	       --------
 ** 24/11/99  	Ashok Roy		    				 Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#define yylval glbSipParserMedialval
#include "siplist.h"
#include "Media.tab.h"
#define YY_DECL int glbSipParserMedialexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Media_fatal_error(msg)
static jmp_buf glbSipParserMediaJumpBuf;
void sip_lex_Media_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}
ALPHA [-a-zA-Z0-9_!*$%^&][-a-zA-Z0-9_!$@<>?%^&*().]*
LWS [ \t]*
CR [\n\r]+
DIGIT [0-9]
HEX [0-9]|[A-F]|[a-f]

ALPHANUM1 ({ALPHA}|{DIGIT})
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
UNRESERVED ({MARK}|{ALPHANUM1}|"/"|"?")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})

EMAIL_SAFE ({RESERVEDNOCOLON}|":"|{UNRESERVED}|{ESCAPED}|"'"|"`"|"#"|"^"|"{"|"}"|"\""|{LWS}|"|")


DIGITS {DIGIT}+
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR ({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS})

%%
[ \t]+ ;
":"	{
	  	SIPDEBUG("SIP PARSER LEXER MEDIA COLON\n");
		return(COLON);
	}
"0"  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA 0\n");
		PASSTOKEN(yytext)
		return(ZERO);
	}
"/"	{
	  	SIPDEBUG("SIP PARSER LEXER MEDIA SLASH\n");
		PASSTOKEN(yytext)
		return(SLASH);
	}
"IN"     {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA NETTYPE\n");
		PASSTOKEN(yytext)
		return(NETTYPE);
	}
"IP"[46] {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA IP\n");
		PASSTOKEN(yytext)
		return(ADDRTYPE);
	}
"prompt" {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA prompt\n");
		PASSTOKEN(yytext)
		return(PROMPT);
	}
"m"({LWS})"=" {return(MEQ);}
"c"({LWS})"=" {return(CEQ);}
"b"({LWS})"=" {return(BEQ);}
"k"({LWS})"=" {return(KEQ);}
"a"({LWS})"=" {return(AEQ);}
[0-9]+  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA DIG\n");
		PASSTOKEN(yytext)
		return(DIG);
	}
[0-9]+"."[0-9]+"."[0-9]+"."[0-9]+  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA IP\n");
		PASSTOKEN(yytext)
		return(IP);
	}
(("clear"({LWS})":"(({LWS}{EMAIL_SAFE})+)|("base64"({LWS})":"(({LWS}{EMAIL_SAFE})+)))) {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA KEY\n");
		PASSTOKEN(yytext)
		return(KEY);
	}
":"({LWS})[0-9]+  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA BAND\n");
		PASSTOKEN(yytext)
		return(BAND);
	}
[a-zA-Z0-9]+  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA ALPHANUM\n");
		PASSTOKEN(yytext)
		return(ALPHANUM);
	}
{IPV6ADDR}	{
	  	SIPDEBUG("SIP PARSER LEXER MEDIA ipv6\n");
		PASSTOKEN(yytext)
		return(IPV6ADDR);
	}
[a-zA-Z][-a-zA-Z0-9]*(("."[a-zA-Z][-a-zA-Z0-9]*)*) {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA host\n");
		PASSTOKEN(yytext)
		return(HOST);
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER MEDIA uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	
[-a-zA-Z][-a-zA-Z0-9!@#$%^&*()._+/]+ {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA PROTO\n");
		PASSTOKEN(yytext)
		return(PROTO);
	}
"uri"({LWS})":"({LWS}){ALPHA}({LWS})":"({LWS})("\/"*)(({ALPHA}|[\.\[\]:~`'@^()$\"/])+)("?"({ALPHA}|[~`'@^()/$\"])+("+"({ALPHA}|[~`'@^()$\"/])+))*("#"([a-zA-Z0-9]|[-!@$%^%&*()_/$\"]|("%"[a-fA-F0-9]{2}))+)? {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA URITEXT\n");
		PASSTOKEN(yytext)
		return(URITXT);
	}
"i"({LWS})"="({LWS})[^\n\r]+        {
		int j=0;
		while(yytext[j]!='=')
			j++;
		PASSTOKEN(&(yytext[j+1]))
	  	SIPDEBUG("SIP PARSER LEXER MEDIA INFO\n");
		return(INFO);
	}
":"[^\n\r]+  {
	  	SIPDEBUG("SIP PARSER LEXER MEDIA BYTESTR\n");
		PASSTOKEN(&(yytext[1]))
		return(BYTESTR);
	}
([^\n\r \t:=\/])+	{
	  	SIPDEBUG("SIP PARSER LEXER MEDIA TOKEN\n");
		PASSTOKEN(yytext)
		return(TOKEN);
	}
{CR} {
		return(CR);
	}
%%
int yywrap(void)
{
 return 1;
}


void sip_lex_Media_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Media_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexMediaMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserMediaJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Media_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexMediaMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Media_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMediaMutex);
#endif

}

int glbSipParserMedialex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserMedialexreal(yylval));
#else
	if (setjmp(glbSipParserMediaJumpBuf) == 0)
	{
		return(glbSipParserMedialexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMediaMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Media_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserMediaJumpBuf,1);
}
#endif

