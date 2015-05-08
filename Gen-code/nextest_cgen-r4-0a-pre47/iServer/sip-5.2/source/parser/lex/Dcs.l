%{
 /***********************************************************************
 ** FUNCTION:
 **             Lex file for DCS headers
 *********************************************************************
 **
 ** FILENAME:
 ** Dcs.l
 **
 ** DESCRIPTION:
 ** This has all the DCS related headers 
 **
 ** DATE        	NAME         		REFERENCE           REASON
 ** ----			----	    		---------	        --------
 ** 13-Nov-2000 	Seshashayi T.							Initial
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Dcs.tab.h"
#define yylval glbSipParserDcslval
#include "siplist.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Dcs_fatal_error(msg)
static jmp_buf glbSipParserDcsJumpBuf;
void sip_lex_Dcs_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
extern  int SetStateDcs;
int lexabort=0;
int glbSipParserDcsAllowSipColon;
#define YY_DECL int glbSipParserDcslexreal(YYSTYPE *lval)
%}
%s AllowQString
%s AllowQString1
%s AllowUserinfo
%s AllowDispName
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
HEX [0-9]|[A-F]|[a-f]
HEXTOKEN {HEX}+
DIGIT [0-9]
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})+
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
EQUALS "="
LWS [ \t]*
SEMICOLON ";"
COLON ":"
COMMA ","
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
ESCAPED "%"{HEX}{HEX}
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
UNRESERVED ({MARK}|{ALPHANUM})
MUNRESERVED {UNRESERVED}+
RESERVEDNOCOLON (";"|"/"|"?"|"@"|"&"|"="|"+"|"$"|","|"["|"]")
URICNOCOLON ({RESERVEDNOCOLON}|{UNRESERVED}|{ESCAPED})
LANGLE {LWS}*"<"
RANGLE ">"
FWSLASH "/"
SEMPARAM ("&"|"$"|"["|"]"|":"|"("|")"|"/")+
SEMPARAMVALUE ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN1 ({SEMPARAM}|{TOKEN})+
SEMPARAMTOKEN ";"{LWS}({SEMPARAMTOKEN1})
SIPCOLON [Ss][Ii][Pp]{LWS}":"
SIPSCOLON [Ss][Ii][Pp][Ss]{LWS}":"
TOKENEQUALS {TOKEN}{LWS}"="
SEMTOKEN ";"{LWS}{TOKEN}
SEMALPHANUM ";"{LWS}{ALPHANUM}
QSTRING1 ("%22"(([^"\\\n])|("\\".))*"%22")
QSTRING "\""(([^"\\\n])|("\\".))*["]
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
QUESTION "?"
DISPLAYNAME ({TOKEN}|{QSTRING}{LWS})*
DISPNAME({TOKEN}|{QSTRING}|{LWS})*"<"
DCSMEDIAAUTH [Pp]-[Mm][Ee][Dd][Ii][Aa]-[Aa][Uu][Tt][Hh][Oo][Rr][Ii][Zz][Aa][Tt][Ii][Oo][Nn]{LWS}":"
DCSANONYMITY [Aa][Nn][Oo][Nn][Yy][Mm][Ii][Tt][Yy]{LWS}":"
DCSREMOTEPARTYID [Rr][Ee][Mm][Oo][Tt][Ee]-[Pp][Aa][Rr][Tt][Yy]-[Ii][Dd]{LWS}":"
DCSRPIDPRIVACY [Rr][Pp][Ii][Dd]-[Pp][Rr][Ii][Vv][Aa][Cc][Yy]{LWS}":"
DCSTRACEPARTYID [Dd][Cc][Ss]-[Tt][Rr][Aa][Cc][Ee]-[Pp][Aa][Rr][Tt][Yy]-[Ii][Dd]{LWS}":"
DCSREDIRECT [Dd][Cc][Ss]-[Rr][Ee][Dd][Ii][Rr][Ee][Cc][Tt]{LWS}":"
DCSSTATE [Ss][Tt][Aa][Tt][Ee]{LWS}":"
DCSGATE [Dd][Cc][Ss]-[Gg][Aa][Tt][Ee]{LWS}":"
DCSLAES [Dd][Cc][Ss]-[Ll][Aa][Ee][Ss]{LWS}":"
DCSSESSION [Ss][Ee][Ss][Ss][Ii][Oo][Nn]{LWS}":"
DCSBILLINGID  [Dd][Cc][Ss]-[Bb][Ii][Ll][Ll][Ii][Nn][Gg]-[Ii][Dd]{LWS}":"
DCSBILLINGINFO [Dd][Cc][Ss]-[Bb][Ii][Ll][Ll][Ii][Nn][Gg]-[Ii][Nn][Ff][Oo]{LWS}":"
DCSOSPS [Dd][Cc][Ss]-[Oo][Ss][Pp][Ss]{LWS}":"
CRLF "\n"|"\r"|"\r\n\r\n"
%%
{LWS} ;
{DCSSTATE}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER DCSSTATE \n"); 
		return DCSSTATE; 
		}
{DCSBILLINGID}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER DCSBILLING ID \n"); 
		return DCSBILLINGID; 
		}
{DCSBILLINGINFO}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER DCSBILLING INFO \n"); 
		return DCSBILLINGINFO; 
		}
{DCSMEDIAAUTH}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER DCSMEDIAAUTH \n"); 
		return DCSMEDIAAUTH; 
	}
{DCSANONYMITY}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSE DCSANONYMITY \n"); 
		return DCSANONYMITY; 
	}
{DCSRPIDPRIVACY}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSE DCSRPIDPRIVACY \n"); 
		return DCSRPIDPRIVACY; 
	}
{DCSREMOTEPARTYID}	{
		BEGIN 0;
		BEGIN AllowDispName ;
		SIPDEBUG("SIP PARSER LEXER DCS dcsremotepartyid\n"); 
		return DCSREMOTEPARTYID; 
	}
{DCSTRACEPARTYID}		{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS TRACEPARTYID\n"); 
		return DCSTRACEPARTYID ;
	}
{DCSREDIRECT}		{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS DCSREDIRECT} \n"); 
		return DCSREDIRECT;
	}
{DCSGATE}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS Gate\n"); 
		return DCSGATE ;
	}
{DCSLAES}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS LAES\n"); 
		return DCSLAES ;
	}
{DCSSESSION}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS SESSION\n"); 
		return DCSSESSION ;
	}
{DCSOSPS}	{
		BEGIN 0;
		SIPDEBUG("SIP PARSER LEXER DCS OSPS\n"); 
		return DCSOSPS ;
	}
{DIGITS} 	{
		SIPDEBUG("SIP PARSER LEXER digits\n");
		PASSTOKEN(yytext);
		BEGIN AllowDispName ;
		return DIGITS;
	}
{HEXTOKEN}		{
		SIPDEBUG("SIP PARSER LEXER DCS hextoken\n");
		PASSTOKEN(yytext);
		return HEXTOKEN;
	}
{ALPHANUM} {
		SIPDEBUG("SIP PARSER LEXER DCS ALPHANUM\n");
		PASSTOKEN(yytext);
		return ALPHANUM;
		}
{FWSLASH} 	{
		SIPDEBUG("SIP PARSER LEXER DCS fwslash\n");
		PASSTOKEN(yytext);
		return FWSLASH ;
	}
	
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER DCS sip:\n");
		if (!glbSipParserDcsAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER DCS starting state userinfoat\n");
			BEGIN AllowUserinfo;
			PASSTOKEN(yytext);
			return SIPCOLON;
		}	
		
		PASSTOKEN(yytext);
		return PLAINSIPCOLON;
	}

{SIPSCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER DCS sips:\n");
		if (!glbSipParserDcsAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER DCS starting state userinfoat\n");
			BEGIN AllowUserinfo;
			PASSTOKEN(yytext);
			return SIPSCOLON;
		}	
		
		PASSTOKEN(yytext);
		return PLAINSIPCOLON;
	}
<AllowQString,AllowQString1>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER DCS qstring\n");
		PASSTOKEN(yytext);
		BEGIN 0;
		return QSTRING;
	}
{QSTRING1}  {
		SIPDEBUG("SIP PARSER LEXER DCS qstring1**\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING1;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER DCS colon\n");
		PASSTOKEN(yytext);
		BEGIN 0 ;
   		return COLON;
	}
{COMMA}		{
	  	SIPDEBUG("SIP PARSER LEXER DCS comma\n");
		PASSTOKEN(yytext);
   		return COMMA;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP_PARSER LEXER DCS semicolon");
		PASSTOKEN(yytext);
		return SEMICOLON;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER DCS equals\n");
		PASSTOKEN(yytext);
   		return EQUALS;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER DCS question\n");
		PASSTOKEN(yytext);
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER DCS right angle\n");
		PASSTOKEN(yytext);
		BEGIN 0;
   		return RANGLE;
	}
		
{LANGLE}	{
		SIPDEBUG("SIP PARSER LEXER DCS left angle\n");
		PASSTOKEN(yytext);
   		return LANGLE;
	}

{TOKEN}		{

		SIPDEBUG("SIP PARSER LEXER DCS pToken\n");
		PASSTOKEN(yytext);
		return TOKEN;
	}

{MUNRESERVED} 	{
		SIPDEBUG("SIP PARSER LEXER DCS unreserved\n");
		PASSTOKEN(yytext);
		return MUNRESERVED;
	}
	
{DISPLAYNAME} 	{
		SIPDEBUG("SIP PARSER LEXER DCS dispname \n");
		PASSTOKEN(yytext);
		return DISPLAYNAME ;
	}

<AllowDispName>{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER DCS dispname \n");
		PASSTOKEN(yytext);
		return DISPNAME ;
	}
<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER DCS userinfoat\n");
		PASSTOKEN(yytext);
		BEGIN 0;
		return USERINFOAT ;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER DCS ipv6addr\n");
		PASSTOKEN(yytext);
		BEGIN 0; 
		return IPV6ADDR ;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER DCS uricnocolon\n");
		PASSTOKEN(yytext);
		return URICNOCOLON ;
	}	
{TOKENEQUALS} {
		SIPDEBUG("SIP PARSER LEXER DCS tokenequals \n");
		PASSTOKEN(yytext);
		return TOKENEQUALS;
	}
	
{SEMALPHANUM} 	{
		SIPDEBUG("SIP PARSER LEXER DCS semalphanum\n");
		PASSTOKEN(yytext);
		BEGIN AllowQString1;
		return SEMALPHANUM;
		}
{SEMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER DCS semtoken\n");
		PASSTOKEN(yytext);
		BEGIN AllowQString1;
		return SEMTOKEN;
		}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER DCS semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{CRLF} {
		SIPDEBUG("SIP PARSER LEXER DCS crlf rule\n");
		PASSTOKEN(yytext);
		return CRLF;
		}
<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER DCS semparamvalue\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return SEMPARAMVALUE;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER DCS default rule\n");
		PASSTOKEN(yytext);
		return DEFAULT;
	}
%%
int yywrap()
{
 return 1;
}
void sip_lex_Dcs_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Dcs_reset_state( void )
{
	glbSipParserDcsAllowSipColon=0;
	BEGIN 0;
}

int sip_lex_Dcs_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexDcsMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserDcsJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Dcs_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexDcsMutex);
#endif
		return 1;
	}
#endif	
	BEGIN AllowDispName ;
}


void sip_lex_Dcs_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexDcsMutex);
#endif

}


int glbSipParserDcslex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserDcslexreal(yylval));
#else
	if (setjmp(glbSipParserDcsJumpBuf) == 0)
	{
		return(glbSipParserDcslexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexDcsMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Dcs_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserDcsJumpBuf,1);
}
#endif

