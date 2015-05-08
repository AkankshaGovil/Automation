%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Resource-Priority headers

 *********************************************************************
 **
 ** FILENAME:
 ** congest.l
 **
 ** DESCRIPTION:
 ** This has all the headers of form pToken/pToken
 **
 ** DATE        NAME         		REFERENCE               REASON
 ** ----	----	     		---------	       --------
 **
 **     Copyright 2003, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Congest.tab.h"
#define yylval glbSipParserCongestlval
#include "siplist.h"
#define YY_DECL int glbSipParserCongestlexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_congest_fatal_error(msg)
static jmp_buf glbSipParserCongestJumpBuf;
void sip_lex_congest_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
DIGITS {DIGIT}+
ALPHANUM ({ALPHA}|{DIGIT})*
LWS [ \t]*
COMMA ","
DOT "."
DASH "-"
RESOURCEPRIORITY ([Rr][Ee][Ss][Oo][Uu][Rr][Cc][Ee]-[Pp][Rr][Ii][Oo][Rr][Ii][Tt][Yy]{LWS}":")
ACCEPTRESOURCEPRIORITY ([Aa][Cc][Cc][Ee][Pp][Tt]-[Rr][Ee][Ss][Oo][Uu][Rr][Cc][Ee]-[Pp][Rr][Ii][Oo][Rr][Ii][Tt][Yy]{LWS}":")

%%
{LWS} ;
{RESOURCEPRIORITY} {
		SIPDEBUG("SIP PARSER LEXER Congest Resource-Priority\n");
		return RESOURCEPRIORITY;
		}
{ACCEPTRESOURCEPRIORITY} {
		SIPDEBUG("SIP PARSER LEXER Congest Accept-Resource-Priority\n");
		return ACCEPTRESOURCEPRIORITY;
		}
{COMMA} {
		SIPDEBUG("SIP PARSER LEXER Congest COMMA\n");
		return COMMA;
        }
{ALPHANUM} 	{
		SIPDEBUG("SIP PARSER LEXER Congest ALPHANUM\n");
        PASSTOKEN(yytext)
		return ALPHANUM;
	}
{DOT} 	{
		SIPDEBUG("SIP PARSER LEXER Congest DOT\n");
		return DOT;
	}
{DASH} 	{
		SIPDEBUG("SIP PARSER LEXER Congest DASH\n");
        PASSTOKEN(yytext)
		return DASH;
	}

%%
int yywrap(void)
{
 return 1;
}

void sip_lex_congest_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_congest_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexCongestMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserCongestJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Congest_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexCongestMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_congest_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexCongestMutex);
#endif
}

int glbSipParserCongestlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserCongestlexreal(yylval));
#else
	if (setjmp(glbSipParserCongestJumpBuf) == 0)
	{
		return(glbSipParserCongestlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexCongestMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_congest_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserCongestJumpBuf,1);
}
#endif

