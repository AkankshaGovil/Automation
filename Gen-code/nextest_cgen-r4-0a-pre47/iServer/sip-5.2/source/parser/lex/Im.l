%{
 /***********************************************************************
 ** FUNCTION:
 **             Lex file for IM-URL

 *********************************************************************
 **
 ** FILENAME:
 ** im.l
 **
 ** DESCRIPTION:
 ** This defines token for parsing IM-URL
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----		----	     		---------	    	   --------
 ** 11/01/02   Manoj Chauhan        Initial					IM-URL supported 
 **
 **     Copyright 2002, Hughes Software Systems, Ltd. 
*********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "imurldecodeintrnl.h"
#include "Im.tab.h"
#include "siplist.h"

#define yylval glbSipParserImlval
#define YY_DECL int glbSipParserImlexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Im_fatal_error(msg)
static jmp_buf glbSipParserImJumpBuf;
void sip_lex_Im_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
%s AllowUrlc
%s AllowDtext
%s AllowImAtom

IMCOLON [Ii][Mm]{LWS}":"
PRESCOLON [Pp][Rr][Ee][Ss]{LWS}":"
LWS [ \t]*
EQUALS "="
COLON ":"
COMMA ","
LANGLE ([%][3][C|c])
RANGLE ([%][3][E|e])
LSQUARE ([%][5][B|b])
RSQUARE ([%][5][D|d])
AT "@"
DOT "."
QUESTION "?"
AMPERSAND "&"
SPACE ([\x25][2][0])
IMATOM ([\x21]|"%23"|[\x24-\x27][\x2a]|[\x2b]|[\x2d]|[\x2f-\x39]|[\x3d]|"%3f"|"%3F"|[\x41-\x5a]|[\x5e-\x7f])*
DTEXT ({LWS}|[\x23-\x24]|[\x26-\x2b]|[\x2d-\x3b]|[\x3d]|[\x3f-\x5a]|[\x5f-\x7a]|[\x7e]|"%00"|"%01"|"%02"|"%03"|"%04"|"%05"|"%06"|"%07"|"%08"|"%09"|"%0a"|"%0A"|"%0b"|"%0B"|"%0c"|"%0C"|"%0e"|"%0E"|"%0f"|"%0F"|"%10"|"%11"|"%12"|"%13"|"%14"|"%15"|"%16"|"%17"|"%18"|"%19"|"%1a"|"%1A"|"%1b"|"%1B"|"%1c"|"%1C"|"%1d"|"%1D"|"%1e"|"%1E"|"%1f"|"%1F"|"%20"|"%21"|"%22"|"%25"|"%2c"|"%2C"|"%3c"|"%3C"|"%3e"|"%3E"|"%5e"|"%5E"|"%7b"|"%7B"|"%7c"|"%7C"|"%7d"|"%7D")*
QPAIRTEXT ([\\]([\x0-\x7f])*)
URLC ([\x21-\x25]|[\x27-\x39]|[\x3b-\x3c]|[\x3e]|[\x40-\x7e])*
QSTRING ("%22"(([^"\\\n])|("\\".))*"%22")
%%
{LWS} ;
{IMCOLON}	{ 
		SIPDEBUG("SIP PARSER LEXER IM-URL imcolon\n");
		BEGIN AllowImAtom;
		return IMCOLON;
	}
{PRESCOLON} {
		SIPDEBUG("SIP PARSER LEXER IM-URL prescolon\n");
		BEGIN AllowImAtom;
		return PRESCOLON;
}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL colon\n");
		PASSTOKEN(yytext)
   		return COLON;
	}
{COMMA}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL comma\n");
		PASSTOKEN(yytext)
   		return COMMA;
	}

{AT}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL at\n");
		PASSTOKEN(yytext)
		BEGIN AllowImAtom;
   		return AT;
	}
{DOT}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL dot\n");
		PASSTOKEN(yytext)
   		return DOT;
	}
{LSQUARE}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL left square bracket\n");
		PASSTOKEN(yytext)
		BEGIN AllowDtext;
   		return LSQUARE;
	}
{RSQUARE}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL right sqaure bracket\n");
		PASSTOKEN(yytext)
		BEGIN AllowImAtom;
   		return RSQUARE;
	}
{AMPERSAND}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL ampersand\n");
		PASSTOKEN(yytext)
   		return AMPERSAND;
	}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER IM-URL equals\n");
		PASSTOKEN(yytext)
		return EQUALS;
	}
{QUESTION}	{
	  	SIPDEBUG("SIP PARSER LEXER IM-URL question\n");
		PASSTOKEN(yytext)
		BEGIN AllowUrlc;
   		return QUESTION;
	}
{SPACE}	{
		SIPDEBUG("SIP PARSER LEXER IM-URL space\n");
		PASSTOKEN(yytext)
		BEGIN AllowImAtom;
   		return SPACE;
	}
{RANGLE}	{
		SIPDEBUG("SIP PARSER LEXER IM-URL right angle\n");
		BEGIN 0;
		PASSTOKEN(yytext)
   		return RANGLE;
	}
{LANGLE}	{
		SIPDEBUG("SIP PARSER LEXER IM-URL left angle\n");
		BEGIN AllowImAtom;
		PASSTOKEN(yytext)
   		return LANGLE;
	}
<AllowImAtom>{IMATOM}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL imatom\n");
		PASSTOKEN(yytext)
   		return IMATOM;
	}
<AllowDtext>{DTEXT}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL dtext\n");
		PASSTOKEN(yytext)
   		return DTEXT;
	}
<AllowUrlc>{URLC}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL urlc\n");
		PASSTOKEN(yytext)
   		return URLC;
	}
{QSTRING}	{
		SIPDEBUG("SIP PARSER LEXER IM-URL qstring\n");
		PASSTOKEN(yytext)
		return QSTRING;
	}
{QPAIRTEXT}		{
		SIPDEBUG("SIP PARSER LEXER IM-URL char\n");
		PASSTOKEN(yytext)
		BEGIN 0;
   		return QPAIRTEXT;
	}

.|\n		{
		SIPDEBUG("SIP PARSER LEXER IM-URL default rule\n");
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}
void sip_lex_Im_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}

void sip_lex_Im_reset_state( void )
{
	BEGIN 0;
}
int sip_lex_Im_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexImMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if (setjmp(glbSipParserImJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Im_scan_buffer "
			"GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexImMutex );
#endif
		return 1;
	}
	
#endif

}

void sip_lex_Im_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexImMutex);
#endif

}

int glbSipParserImlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserImlexreal(yylval));
#else
	if (setjmp(glbSipParserImJumpBuf) == 0)
	{
		return(glbSipParserImlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexImMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Im_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserImJumpBuf,1);
}
#endif

