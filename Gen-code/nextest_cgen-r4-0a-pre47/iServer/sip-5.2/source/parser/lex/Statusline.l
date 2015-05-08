%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Status line

 *********************************************************************
 **
 ** FILENAME:
 ** utf8.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----	----	     		---------	       --------
 ** 24/11/99  KSBinu, Arjun RC			         Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Statusline.tab.h" 
#define yylval glbSipParserStatuslinelval
#include "siplist.h"
#define YY_DECL int glbSipParserStatuslinelexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Statusline_fatal_error(msg)
static jmp_buf glbSipParserStatuslineJumpBuf;
void sip_lex_Statusline_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
DIGIT [0-9]
DIGIT3 ((" "|"\t"){DIGIT}{3,3})((" "|"\t")*)
SIPFLOAT ({DIGIT}+)"."({DIGIT}+)
LWS [ \t]*
TEXTUTF8NOSLASHDIGITS ([\x20-\x2e]|[\x3a-\x7f]|" "|"\t")+
SLASH "/"
SIPSLASH [Ss][Ii][Pp]{LWS}"/"
LINEEND "\n"|"\r"|"\r\n"
%%
{LWS} ;
{SIPSLASH} 	{	
		SIPDEBUG("SIP PARSER LEXER STATUSLINE sipslash\n");
		PASSTOKEN(yytext)
		return SIPSLASH;
	}
{DIGIT}		{
		SIPDEBUG("SIP PARSER LEXER STATUSLINE digit\n");
		PASSTOKEN(yytext)
		return DIGIT;
	}
{DIGIT3}	{
		SIPDEBUG("SIP PARSER LEXER STATUSLINE digit3\n");
		PASSTOKEN(yytext)
		return DIGIT3;
	}
{SIPFLOAT}		{
		SIPDEBUG("SIP PARSER LEXER STATUSLINE float\n");
		PASSTOKEN(yytext)
		return SIPFLOAT;
	}
{TEXTUTF8NOSLASHDIGITS} {
		SIPDEBUG("SIP PARSER LEXER STATUSLINE textutf8noslashdigits\n");
		PASSTOKEN(yytext)
		return TEXTUTF8NOSLASHDIGITS;
	}
{SLASH}		{
		SIPDEBUG("SIP PARSER LEXER STATUSLINE slash\n");
		PASSTOKEN(yytext)
   		return SLASH;
	}
{LINEEND}	{
		SIPDEBUG("SIP PARSER LEXER REQLINE lineend \n");
		return LINEEND;
	}
.|\n 	{
			return DEFAULT;
		}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_Statusline_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}


int sip_lex_Statusline_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexStatusMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserStatuslineJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Statusline_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexStatusMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Statusline_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexStatusMutex);
#endif

}

int glbSipParserStatuslinelex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserStatuslinelexreal(yylval));
#else
	if (setjmp(glbSipParserStatuslineJumpBuf) == 0)
	{
		return(glbSipParserStatuslinelexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexStatusMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Statusline_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserStatuslineJumpBuf,1);
}
#endif

