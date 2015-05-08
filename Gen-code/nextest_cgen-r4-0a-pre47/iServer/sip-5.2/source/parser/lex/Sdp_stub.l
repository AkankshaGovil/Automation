%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for SDP

 *********************************************************************
 **
 ** FILENAME:
 ** tokensltoken.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE          REASON
 ** ----		----	     		---------	      --------
 ** 24/11/99   Ashok Roy		  Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/

#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Sdp.tab.h"
#define yylval glbSipParserSdplval
#include "siplist.h"
#define YY_DECL int glbSipParserSdplexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Sdp_fatal_error(msg)
#define YY_BEGIN_URIC(void) sip_lex_Sdp_begin_uric(coid)
#define YY_BEGIN_URI(void) sip_lex_Sdp_begin_uri_allow(coid)
static jmp_buf glbSipParserSdpJumpBuf;
void sip_lex_Sdp_fatal_error(const SIP_S8bit *pErrMsg); 
void sip_lex_Sdp_begin_uric(void);
void sip_lex_Sdp_begin_uri_allow(void);
#endif

%}
%s AllowSafe
%s AllowURI
%s AllowURIC
%s AllowPhone
ALPHA [-a-zA-Z0-9_!*$%^&][-a-zA-Z0-9_!$%^&*().]* 
ALPHASAFE ([-'`./:?"#$^&*;=@_|{}+~a-zA-Z0-9]|"["|"]")*
COMMENT ("("[^\n]*")")
AQ3 \"(([^"\\\n])|("\\".))*["]
LWS [ \t]*
CR [\n\r]+
CLEARSAFE "clear:"[ \t]|"base64:"[ \t] 

DIGIT [0-9]
DIGITS {DIGIT}+
HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR ({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS})

ALPHANUM ({ALPHA}|{DIGIT})
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
UNRESERVED ({MARK}|{ALPHANUM}|"/"|"?")
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
URICOLON "uri"{LWS}":"
EMAIL_SAFE ({RESERVEDNOCOLON}|":"|{UNRESERVED}|{ESCAPED}|"'"|"`"|"#"|"^"|"{"|"}"|"\""|{LWS}|"|")

SCHEME {ALPHA}({LWS})":"
HEADERS ("?"({ALPHA}|[~=;$,&?+`@^()/])*)
FRAGMENT ("#"([a-zA-Z0-9]|[-!@$%^%&*()_/]|("%"[a-fA-F0-9]{2}))+)
PATH1 (({ALPHA}|[~`@^()/:\[\]])+)
LENIENT ([~`@^()/:\[\]]|"`"|"'"|"?"|";"|"=")
SAFE ("$"|"-"|"_"|"$"|"."|"&"|"@"|{LENIENT})
EXTRA ("!"|"*"|"\""|"`"|"("|")"|",")
XALPHA ({ALPHANUM}|{SAFE}|{EXTRA}|{ESCAPED})
PATH ({XALPHA})+
URI {SCHEME}({LWS})("\/"*)({PATH})({HEADERS})*({FRAGMENT})?
%%
{CLEARSAFE}  {
	  	SIPDEBUG("SIP PARSER LEXER SDP clearsafe\n");
		PASSTOKEN(yytext)
		return(CLEARSAFE);
	}
[ \t]+ ;
"," ;
"0"  {
	  	SIPDEBUG("SIP PARSER LEXER SDP 0\n");
		PASSTOKEN(yytext)
		return(ZERO);
	}
(IP[46]) {
	  	SIPDEBUG("SIP PARSER LEXER SDP IP46\n");
		PASSTOKEN(yytext)
		return(ADDRTYPE);
	}
{URICOLON} {
	  	SIPDEBUG("SIP PARSER LEXER SDP URICOLON\n");
		PASSTOKEN(yytext)
		return(URICOLON);
	}
"IN" {
	  	SIPDEBUG("SIP PARSER LEXER SDP IN\n");
		PASSTOKEN(yytext)
		return(NETTYPE);
	}
[0-9]+[dhms]      {
	  	SIPDEBUG("SIP PARSER LEXER SDP DHMS\n");
		PASSTOKEN(yytext)
		return(DHMS);
	}
"/"	{
	  	SIPDEBUG("SIP PARSER LEXER SDP SLASH\n");
		PASSTOKEN(yytext)
		return(SLASH);
	}
"@"         {
	  	SIPDEBUG("SIP PARSER LEXER SDP AT\n");
		PASSTOKEN(yytext)
		return(AT);
	}
"-"([0-9]+)([dhms]?) {
	  	SIPDEBUG("SIP PARSER LEXER SDP DASHDIG\n");
		PASSTOKEN(yytext)
		return(DASHDIG);
	}
":" 	    {
	  	SIPDEBUG("SIP PARSER LEXER SDP COLON\n");
		PASSTOKEN(yytext)
		return(COLON);
	}
"v"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP v=\n");
		return(VEQ);}
"o"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP o=\n");
		BEGIN AllowSafe;
		return(OEQ);}
"u"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP u=\n");
		BEGIN AllowURI;
		return(UEQ);}
"e"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP e=\n");
		return(EEQ);}
"p"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP p=\n");
		BEGIN AllowPhone ;
		return(PEQ);}
"c"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP c=\n");
		return(CEQ);}
"b"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP b=\n");
		return(BEQ);}
"t"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP t=\n");
		return(TEQ);}
"r"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP r=\n");
		return(REQ);}
"k"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP k=\n");
		return(KEQ);}
"z"({LWS})"=" {
	  	SIPDEBUG("SIP PARSER LEXER SDP z=\n");
		return(ZEQ);}
([1-9][0-9]*) {
	  	SIPDEBUG("SIP PARSER LEXER SDP start\n");
		PASSTOKEN(yytext)
		return(START);
	}
[0-9]+ {
	  	SIPDEBUG("SIP PARSER LEXER SDP dig\n");
		PASSTOKEN(yytext)
		return(DIG);
	}
[0-9]+"."[0-9]+"."[0-9]+"."[0-9]+ {
	  	SIPDEBUG("SIP PARSER LEXER SDP ip\n");
		PASSTOKEN(yytext)
		return(IP);
	}
(("prompt")|("clear"({LWS})":"(({LWS}{EMAIL_SAFE})+)|("base64"({LWS})":"(({LWS}{EMAIL_SAFE})+)))) {
	  	SIPDEBUG("SIP PARSER LEXER SDP key\n");
		PASSTOKEN(yytext)
		return(KEY);
	}
<AllowSafe>{ALPHASAFE} {
			BEGIN 0;
	  	SIPDEBUG("SIP PARSER LEXER SDP alphasafe\n");
		PASSTOKEN(yytext)
		return(ALPHASAFE);
	}
[a-zA-Z][-a-zA-Z0-9]*(("."[a-zA-Z][-a-zA-Z0-9]*)*) {
	  	SIPDEBUG("SIP PARSER LEXER SDP  host\n");
		PASSTOKEN(yytext)
		return(HOST);
	}
<AllowURIC>{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER SDP uricnocolon\n");
		BEGIN 0;
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	
[-a-zA-Z][-a-zA-Z0-9!@#$%^.&*()_+/]+ {
	  	SIPDEBUG("SIP PARSER LEXER SDP proto\n");
		PASSTOKEN(yytext)
		return(PROTO);
	}
<AllowURI>{URI} {
		SIPDEBUG("SIP PARSER LEXER SDP uri\n");
		BEGIN 0;
		PASSTOKEN(yytext)
		return(URI);
	}
{IPV6ADDR} {
	  	SIPDEBUG("SIP PARSER LEXER SDP ipv6\n");
		PASSTOKEN(yytext)
		return(IPV6ADDR);
	}
({ALPHA})"@"([-a-zA-Z0-9_]+)(("."[-a-zA-Z0-9_]+)*)({LWS})(({COMMENT})?) {
	  	SIPDEBUG("SIP PARSER LEXER SDP EMAIL\n");
		PASSTOKEN(yytext)
		return(EMAIL);
	}
(({AQ3})|(({ALPHA}{LWS})+))({LWS})"<"({LWS})({ALPHA}"@"{ALPHA})({LWS})">" {
	  	SIPDEBUG("SIP PARSER LEXER SDP EMAIL1\n");
		PASSTOKEN(yytext)
		return(EMAIL1);
	}
(([a-zA-Z0-9]|"-")+)({LWS})":"({LWS})[0-9]+  {
	  	SIPDEBUG("SIP PARSER LEXER SDP BAND\n");
		PASSTOKEN(yytext)
		return(BAND);
	}
<AllowPhone>("+"[1-9])(({LWS}|(("-")+)|([0-9]))+){LWS}(({COMMENT})?) {
		BEGIN 0 ;
	  	SIPDEBUG("SIP PARSER LEXER SDP PHONE\n");
		PASSTOKEN(yytext)
		return(PHONE);
	}

<AllowPhone>((({ALPHASAFE}{LWS})+){LWS}"<"{LWS}("+"[1-9])(({LWS}|(("-")+)|([0-9]))+){LWS}">") {
		BEGIN 0 ;
	  	SIPDEBUG("SIP PARSER LEXER SDP PHONE\n");
		PASSTOKEN(yytext)
		return(PHONE);
	}
"i"({LWS})"="({LWS})[^\n\r]+ {
		int j=0;
		while(yytext[j]!='=')
			j++;
		PASSTOKEN(&(yytext[j+1]))
	  	SIPDEBUG("SIP PARSER LEXER SDP i=\n");
		return(IEQ);
	}
"s"({LWS})"="({LWS})[^\n\r]+ {
		int j=0;
		while(yytext[j]!='=')
			j++;
		PASSTOKEN(&(yytext[j+1]))
	  	SIPDEBUG("SIP PARSER LEXER SDP s=\n");
		return(SEQ);
	}
":"[^\n\r]+  {
	  	SIPDEBUG("SIP PARSER LEXER SDP BYTESTR\n");
		PASSTOKEN(&(yytext[1]))
		return(BYTESTR);
	}
([^\n\r \t:=\/])+	{
	  	SIPDEBUG("SIP PARSER LEXER SDP TOKEN\n");
		PASSTOKEN(yytext)
		return(TOKEN);
	}
{CR} {return(CR);}
.|\n 	{
			return DEFAULT;
		}
%%
int yywrap(void)
{
	return(1);
}
void sip_lex_Sdp_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Sdp_reset_state(void)
{
	BEGIN 0;
}

void sip_lex_Sdp_begin_uric(void)
{
	BEGIN AllowURIC;
}

void sip_lex_Sdp_begin_uri_allow(void)
{
	BEGIN AllowURI;
}

int sip_lex_Sdp_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexSdpMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserSdpJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Sdp_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexSdpMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Sdp_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexSdpMutex);
#endif
}

int glbSipParserSdplex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserSdplexreal(yylval));
#else
	if (setjmp(glbSipParserSdpJumpBuf) == 0)
	{
		return(glbSipParserSdplexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexSdpMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Sdp_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserSdpJumpBuf,1);
}
#endif

