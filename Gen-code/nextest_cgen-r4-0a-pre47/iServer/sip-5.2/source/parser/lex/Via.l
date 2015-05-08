%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Via headers

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
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"
#include "Via.tab.h" 
#define yylval glbSipParserVialval
#include "siplist.h"
void viacommentparse(char *pText, YYSTYPE *lval);
#define YY_DECL int glbSipParserVialexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Via_fatal_error(msg)
static jmp_buf glbSipParserViaJumpBuf;
void sip_lex_Via_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
%s AllowQString
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
DIGITS {DIGIT}+
SIPFLOAT ({DIGIT}+)"."({DIGIT}+)
ALPHANUM ({ALPHA}|{DIGIT})

/* NEXTONE - added "#" */
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`"|"#")+

HEX [0-9]|[A-F]|[a-f]
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
LWS [ \t]*
COMMENT "("((.|[ \t])*)")"
SEMICOLON ";"
COMMA ","
COLON ":"
EQUALS "="
SLASH "/"
SEMTOKEN ";"{LWS}{TOKEN}
QSTRING "\""(([^"\\\n])|("\\".))*["]
VIAF ([Vv][Ii][Aa]){LWS}":"
VIAS ([Vv]){LWS}":"
SECURITYCLIENT ([Ss][Ee][Cc][Uu][Rr][Ii][Tt][Yy][-][Cc][Ll][Ii][Ee][Nn][Tt]){LWS}":"
SECURITYSERVER ([Ss][Ee][Cc][Uu][Rr][Ii][Tt][Yy][-][Se][Ee][Rr][Vv][Ee][Rr]){LWS}":"
SECURITYVERIFY ([Ss][Ee][Cc][Uu][Rr][Ii][Tt][Yy][-][Vv][Ee][Rr][Ii][Ff][Yy]){LWS}":"
%%
{LWS} ;
{VIAS}	{
		SIPDEBUG("SIP PARSER LEXER VIA via\n");
		PASSCHAR('s')
		return VIA;
	}
{VIAF}	{
		SIPDEBUG("SIP PARSER LEXER VIA via\n");
		PASSCHAR('f')
		return VIA;
	}
{SECURITYCLIENT} {
		SIPDEBUG("SIP PARSER LEXER SECURITYCLIENT\n");
		return SECURITYCLIENT;	
	}
{SECURITYSERVER} {
                SIPDEBUG("SIP PARSER LEXER SECURITYSERVER\n");
                return SECURITYSERVER; 
        }

{SECURITYVERIFY} {
                SIPDEBUG("SIP PARSER LEXER SECURITYVERIFY\n");
                return SECURITYVERIFY; 
        }
{DIGITS}	{
		PASSTOKEN(yytext)
		SIPDEBUG("SIP PARSER LEXER VIA Received digits\n");
		return DIGITS;
	}
{SLASH}		{
		SIPDEBUG("SIP PARSER LEXER VIA slash\n");
   		return SLASH;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER VIA equals\n");
   		return EQUALS;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER VIA colon\n");
   		return COLON;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER VIA comma\n");
   		return COMMA;
	}
{COMMENT}	{
		SIPDEBUG("SIP PARSER LEXER VIA Comment\n");
		viacommentparse (yytext, lval);
		return COMMENT;
	}
<AllowQString>{QSTRING} {
		SIPDEBUG("SIP PARSER LEXER VIA qstring\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return QSTRING;
	}
{IPV6ADDR}  {     
		SIPDEBUG("SIP PARSER LEXER REQLINE ipv6addr\n");
		PASSTOKEN(yytext)
		BEGIN 0;
		return IPV6ADDR ;
	}
{SIPFLOAT}		{
		SIPDEBUG("SIP PARSER LEXER Via float\n");
		PASSTOKEN(yytext)
		return SIPFLOAT;
	}
{TOKEN}		{
		SIPDEBUG("SIP PARSER LEXER VIA Token \n");
		PASSTOKEN(yytext)
	 	return TOKEN;
	}
{SEMTOKEN}	{
		SIPDEBUG("SIP PARSER LEXER VIA semtoken\n");
		PASSTOKEN(yytext)
		BEGIN AllowQString;
		return SEMTOKEN;
	}
.|\n	{
		SIPDEBUG("SIP PARSER LEXER VIA default rule\n");
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
void viacommentparse(char* myvar, YYSTYPE *lval)
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
void sip_lex_Via_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}
void sip_lex_Via_reset_state(void)
{
	BEGIN 0;
}

int sip_lex_Via_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexViaMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserViaJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Via_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexViaMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Via_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexViaMutex);
#endif
}

int glbSipParserVialex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserVialexreal(yylval));
#else
	if (setjmp(glbSipParserViaJumpBuf) == 0)
	{
		return(glbSipParserVialexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexViaMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Via_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserViaJumpBuf,1);
}
#endif

