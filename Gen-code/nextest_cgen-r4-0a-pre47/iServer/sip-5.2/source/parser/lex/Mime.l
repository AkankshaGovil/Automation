%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for mime headers

 *********************************************************************
 **
 ** FILENAME:
 ** mime.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE   	     NAME         		REFERENCE          REASON
 ** ----		----	     		---------	       --------
 ** 11/02/2000  Preethy.R						     Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "siplist.h"
#include "sipdecodeintrnl.h"
#include "Mime.tab.h"
#define yylval glbSipParserMimelval
#include "siplist.h"
#define YY_DECL int glbSipParserMimelexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Mime_fatal_error(msg)
static jmp_buf glbSipParserMimeJumpBuf;
void sip_lex_Mime_fatal_error(const SIP_S8bit *pErrMsg); 
#endif

%}
%s AllowHeaderBody
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
DIGIT3 {DIGIT}{3,3}
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
SLASH "/"
SEMICOLON ";"
EQUALS "="
LWS [ \t]*
COLON ":"
COMMA ","
DQUOTE "\""
LANGLE "<"
RANGLE ">"
AT "@"
SIPTEXT ([^\r]|([\r]([^\n]*)))*
CRLF [\r][\n]
BODYSEPARATOR [\r][\n][\r][\n]
ADDRSPEC [.]*
CONTENTTYPE [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Tt][Yy][Pp][Ee]{LWS}{COLON}
CONTENTTRANSENCODING [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Tt][Rr][Aa][Nn][Ss][Ff][Ee][Rr]-[Ee][Nn][Cc][Oo][Dd][Ii][Nn][Gg]{LWS}{COLON}
CONTENTDESCRIPTION [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Dd][Ee][Ss][Cc][Rr][Ii][Pp][Tt][Ii][Oo][Nn]{LWS}{COLON}
CONTENTID [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Ii][Dd]{LWS}{COLON}
QSTRING "\""(([^"\\\n])|("\\".))*["]
LRANGE ({ALPHA}{1,8}("-"({ALPHA}{1,8}))*)|"*"
HEADERNAME ({TOKEN}{LWS}{COLON})
HEADERBODYNOCOLON [^\n\r:]*
CONTENTDISPOSITION [Cc][Oo][Nn][Tt][Ee][Nn][Tt]-[Dd][Ii][Ss][Pp][Oo][Ss][Ii][Tt][Ii][Oo][Nn]{LWS}":"
%%
{LWS} ;
{CRLF}		{
		SIPDEBUG("SIP PARSER LEXER MIME crlf\n");
		BEGIN 0;
		return CRLF;
	}
{CONTENTID} {
		SIPDEBUG("SIP PARSER LEXER MIME contentid\n");
		return CONTENTID;
	}
{BODYSEPARATOR} {
		SIPDEBUG("SIP PARSER LEXER MIME bodyseparator\n");
		return BODYSEPARATOR;
	}
{CONTENTDISPOSITION}	{
		SIPDEBUG("SIP PARSER LEXER MIME content-disposition\n");
		return CONTENTDISPOSITION;
	}
{CONTENTTYPE}	{
		SIPDEBUG("SIP PARSER LEXER MIME contenttype\n");
		return CONTENTTYPE;
	}
{CONTENTTRANSENCODING}	{
		SIPDEBUG("SIP PARSER LEXER MIME contenttransencoding\n");
		return CONTENTTRANSENCODING;
	}
{CONTENTDESCRIPTION}	{
		SIPDEBUG("SIP PARSER LEXER MIME contentdescription\n");
		return CONTENTDESCRIPTION;
	}
{DQUOTE}	{
		SIPDEBUG("SIP PARSER LEXER MIME dquote\n");
   		return DQUOTE;
	}
{SEMICOLON}	{
		SIPDEBUG("SIP PARSER LEXER MIME semicolon\n");
   		return SEMICOLON;
	}
{EQUALS}	{
		SIPDEBUG("SIP PARSER LEXER MIME equals\n");
   		return EQUALS;
	}
{COMMA}	{
		SIPDEBUG("SIP PARSER LEXER MIME comma\n");
   		return COMMA;
	}
{COLON}	{
		SIPDEBUG("SIP PARSER LEXER MIME colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{LANGLE} {
		SIPDEBUG("SIP PARSER LEXER MIME LANGLE\n");
   		return LANGLE;
	}

{RANGLE} {
		SIPDEBUG("SIP PARSER LEXER MIME RANGLE\n");
   		return RANGLE;
	}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER MIME Token\n");
		PASSTOKEN(yytext)
		return TOKEN;
	}
{SLASH}		{
		SIPDEBUG("SIP PARSER LEXER MIME slash\n");
 		return SLASH;
	}
{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER MIME quoted string\n");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{HEADERNAME} 	{
		SIPDEBUG("SIP PARSER LEXER MIME headername\n");
		PASSTOKEN(yytext)
		BEGIN AllowHeaderBody;
		return HEADERNAME;
	}
<AllowHeaderBody>{HEADERBODYNOCOLON} 	{
		SIPDEBUG("SIP PARSER LEXER MIME headerbodynocolon\n");
		PASSTOKEN(yytext)
		return HEADERBODYNOCOLON;
	}
.|\n	{
		SIPDEBUG("SIP PARSER LEXER MIME default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_Mime_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Mime_reset_state(void)
{
	BEGIN 0;
}

int sip_lex_Mime_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexMimeMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserMimeJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Mime_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexMimeMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Mime_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMimeMutex);
#endif

}

int glbSipParserMimelex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserMimelexreal(yylval));
#else
	if (setjmp(glbSipParserMimeJumpBuf) == 0)
	{
		return(glbSipParserMimelexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexMimeMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Mime_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserMimeJumpBuf,1);
}
#endif

