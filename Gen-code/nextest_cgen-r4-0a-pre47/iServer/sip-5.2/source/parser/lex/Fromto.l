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
#include "Fromto.tab.h"
#include "siplist.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Fromto_fatal_error(msg)
static jmp_buf glbSipParserFromtoJumpBuf;
void sip_lex_Fromto_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
int glbSipParserAllowHeaderParams;
int glbSipParserFromToAllowSipColon;
int glbSipAssertedIdNameAddrStarted=0;
int glbSipPPreferredIdNameAddrStarted=0;
int glbSipParserPAssertIdHeaderStarted=0;
int glbSipParserPPreferredIdHeaderStarted=0;
extern int glbSipParserAllowSipUrlParams;
extern SipReferredByHeader * glbSipParserReferredByHeader;
#define YY_DECL int glbSipParserFromtolexreal(YYSTYPE *lval)
#define yylval glbSipParserFromtolval
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
CID	";"{LWS}[Cc][Ii][Dd]
ALPHANUM ({ALPHA}|{DIGIT})

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`"|"#")+

EQUALS "="
QUOTE "\""
LWS [ \t]*
SEMICOLON ";"
COLON ":"
COMMA [,]+

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
USERPASS ({UNRESERVED}|{ESCAPED}|"&"|"="|"+"|"$"|","|";"|"%")+
USERINFOAT {USERPASS}((":"{USERPASS}){0,1})"@"
QUESTION "?"
DISPNAME ({TOKEN}|{QSTRING}|{LWS})*"<"
FROMF ([Ff][Rr][Oo][Mm]){LWS}":"
FROMS ([Ff]){LWS}":"
TOF ([Tt][Oo]){LWS}":"
TOS ([Tt]){LWS}":"
ROUTE [Rr][Oo][Uu][Tt][Ee]{LWS}":" 
RECORDROUTE [Rr][Ee][Cc][Oo][Rr][Dd]-[Rr][Oo][Uu][Tt][Ee]{LWS}":"
PATH [Pp][Aa][Tt][Hh]{LWS}":"
SERVICEROUTE [Ss][eE][rR][vV][iI][Cc][Ee]-[Rr][oO][uU][tT][eE]{LWS}":"
REFERTOS [rR]{LWS}":"
REFERTOF [rR][Ee][Ff][Ee][rR]-[Tt][oO]{LWS}":"
REFERBYS [bB]{LWS}":"
REFERBYF [rR][Ee][Ff][Ee][rR][rR][Ee][Dd]-[bB][yY]{LWS}":"
PANINFO [Pp]-[Aa][Cc][Cc][Ee][Ss][Ss]-[Nn][Ee][Tt][Ww][Oo][Rr][Kk]-[Ii][Nn][Ff][Oo]{LWS}":"
PCVECTOR [Pp]-[Cc][Hh][Aa][Rr][Gg][Ii][Nn][Gg]-[Vv][Ee][Cc][Tt][Oo][Rr]{LWS}":"
PASSERTEDID [pP][-][aA][sE][sS][eE][rR][tT][eE][dD][-][iI][dD][eE][nN][tT][iI][tT][yY]{LWS}":"
PPREFERREDID [pP][-][pP][rR][eE][fF][eE][rR][rR][eE][dD][-][iI][dD][eE][nN][tT][iI][tT][yY]{LWS}":"
%%
{LWS} ;
{REFERBYS}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO b:\n");
		PASSCHAR('s')
		return REFERBY;
	}
{REFERBYF}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO Referred-By:\n");
		PASSCHAR('f')
		return REFERBY;
	}
{REFERTOS}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO r:\n");
		PASSCHAR('s')
		return REFERTO;
	}
{REFERTOF}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO refer to:\n");
		PASSCHAR('f')
		return REFERTO;
	}
{FROMS}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO f:\n");
		PASSCHAR('s')
		return FROM;
	}
{PANINFO}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER PANINFO :\n");
		return PANINFO;
	}
{PCVECTOR}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER PCVECTOR:\n");
		PASSCHAR('s')
		return PCVECTOR;
	}
	
{FROMF}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO from:\n");
		PASSCHAR('f')
		return FROM;
	}
{TOF}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO To:\n");
		PASSCHAR('f')
		return TO;
	}
{TOS}		{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO T:\n");
		PASSCHAR('s')
		return TO;
	}
{PPREFERREDID} {
	    BEGIN 0;
	    SIPDEBUG("SIP PARSER LEXER PPREFERRED IDENTITY\n");
		glbSipParserPPreferredIdHeaderStarted=1;
	    return PPREFERREDID ;
    }
{PASSERTEDID} {
	    BEGIN 0;
	    SIPDEBUG("SIP PARSER LEXER PASSERTED IDENTITY\n");
		glbSipParserPAssertIdHeaderStarted=1;
	    return PASSERTEDID ;
    }
{ROUTE}		{ 	
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO Route\n");
		return ROUTE;
	}
{RECORDROUTE}	{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO RecordRoute\n");
		return RECORDROUTE;
	}
{PATH}	{ 
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FFROMTO Path\n");
		return PATH;
	}
{SERVICEROUTE} {
		BEGIN 0;
		glbSipParserAllowHeaderParams=1;
		glbSipParserAllowSipUrlParams=0;
		SIPDEBUG("SIP PARSER LEXER FROMTO SERVICEROUTE\n");
		return SERVICEROUTE;
  }
{TAG}	{ 
		SIPDEBUG("SIP PARSER LEXER FFROMTO Tag\n");
		PASSTOKEN(yytext)
		return TAG;
	}
{QUOTE} {
		SIPDEBUG("SIP PARSER LEXER FFROMTO QUOTE\n");
		BEGIN AllowUserinfo;
		return QUOTE ;
}
{REFTOKEN}	{
	  	SIPDEBUG("SIP PARSER LEXER FFROMTO Ref\n");
		PASSTOKEN(yytext)
		BEGIN 0;
   		return REFTOKEN;
}
{SIPCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER FFROMTO sip:\n");
		if (!glbSipParserFromToAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER FFROMTO starting state userinfoat\n");
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
		SIPDEBUG("SIP PARSER LEXER FFROMTO sip:\n");
		if (!glbSipParserFromToAllowSipColon)
		{	
			SIPDEBUG("SIP PARSER LEXER FFROMTO starting state userinfoat\n");
			BEGIN AllowUserinfo;
			return SIPSCOLON;
		}	
		else
		{
			PASSTOKEN(yytext)
			return PLAINSIPCOLON;
		}
	}

{CID}	{
			if (glbSipParserReferredByHeader==SIP_NULL)
			{	
				SIPDEBUG("SIP PARSER LEXER FFROMTO semtoken\n");
				PASSTOKEN(yytext)
				BEGIN AllowQString1;
				BEGIN AllowUserinfo;
				return SEMTOKEN;
			}
			else
			{
				SIPDEBUG("SIP PARSER LEXER FFROMTO cid\n");
				return CID;
			}
		}	

<AllowQString,AllowQString1>{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{QSTRING1}  {
		SIPDEBUG("SIP PARSER LEXER FFROMTO qstring1\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING1;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER FFROMTO colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{COMMA}		{
		PASSTOKEN(yytext)
		BEGIN 0;
		if ( ((glbSipParserPAssertIdHeaderStarted) || (glbSipParserPPreferredIdHeaderStarted))
				&&
			(!glbSipAssertedIdNameAddrStarted) )
		{	
	 	 	SIPDEBUG("SIP PARSER LEXER FFROMTO separator\n");
			return SEPARATOR;
		}	
			else
		{		
	 	 	SIPDEBUG("SIP PARSER LEXER FFROMTO comma\n");
	   		return COMMA;
		}	
	}
{SEMICOLON}	{
		SIPDEBUG("SIP_PARSER LEXER FROMTO semicolon");
		PASSTOKEN(yytext)
		return SEMICOLON;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER FFROMTO equals\n");
		PASSTOKEN(yytext)
   		return EQUALS;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER FFROMTO question\n");
		PASSTOKEN(yytext)
   		return QUESTION;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO right angle\n");
		glbSipParserAllowHeaderParams = 1;
		glbSipParserAllowSipUrlParams = 0;
   		return RANGLE;
	}
{DIGITS} 	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER FFROMTOfromto digits\n");
		return DIGITS ;
	}
	
{DISPNAME} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO dispname \n");
		PASSTOKEN(yytext)
		glbSipParserAllowHeaderParams = 0;
		glbSipParserAllowSipUrlParams = 1;
		return DISPNAME ;
	}
		
<AllowUserinfo>{USERINFOAT} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO userinfoat\n");
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
		SIPDEBUG("SIP PARSER LEXER FFROMTO semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMTOKEN;
		}
{SEMPARAMTOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO semparamtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString1;
		return SEMPARAMTOKEN;
		}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO pToken\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return TOKEN;
	}
{URICNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO uricnocolon\n");
		PASSTOKEN(yytext)
		return URICNOCOLON ;
	}	

<AllowQString1>{SEMPARAMVALUE} 	{
		SIPDEBUG("SIP PARSER LEXER FFROMTO semparamvalue\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return SEMPARAMVALUE;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER FFROMTO default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_Fromto_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Fromto_reset_state( void )
{
	BEGIN 0;
	glbSipParserFromToAllowSipColon=0;
	glbSipParserPAssertIdHeaderStarted=0;
	glbSipParserPPreferredIdHeaderStarted=0;
 	glbSipAssertedIdNameAddrStarted=0;
}
int sip_lex_Fromto_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexFromtoMutex, FAST_MXLOCK_EXCLUSIVE );
#endif
	BEGIN 0;
#ifdef SIP_NOFLEXRECOVERY
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if (setjmp(glbSipParserFromtoJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Fromto_scan_buffer "
			"GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexFromtoMutex );
#endif
		return 1;
	}
	
#endif

}
void sip_lex_Fromto_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexFromtoMutex);
#endif

}

int glbSipParserFromtolex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserFromtolexreal(yylval));
#else
	if (setjmp(glbSipParserFromtoJumpBuf) == 0)
	{
		return(glbSipParserFromtolexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexFromtoMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Fromto_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserFromtoJumpBuf,1);
}
#endif

