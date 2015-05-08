%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for UTF8 headers

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
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"
#include "Utf8.tab.h" 
#define yylval glbSipParserUtf8lval
#include "siplist.h"
#define YY_DECL int glbSipParserUtf8lexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Utf8_fatal_error(msg)
static jmp_buf glbSipParserUtf8JumpBuf;
void sip_lex_Utf8_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
LWS [ \t]*
SUBJECTF ([Ss][Uu][Bb][Jj][Ee][Cc][Tt]){LWS}":"
SUBJECTS ([Ss]){LWS}":"
ORGANIZATION (([Oo][Rr][Gg][Aa][Nn][Ii][Zz][Aa][Tt][Ii][Oo][Nn]){LWS}":")
COLON ":"
TEXTUTF8NOCOLON ({TEXT-UTF8char}+)(({LWS}{TEXT-UTF8char})*)
TEXT-UTF8char ([\x21-\x39]|[\x3b-\x7f])|{UTF8-NONASCII}
UTF8-NONASCII  ([\xc0-\xdf]{UTF8-CONT})|([\xe0-\xef]{UTF8-CONT}{UTF8-CONT})|([\xf0-\xf7]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})|([\xf8-\xfb]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})|([\xfc-\xfd]{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT}{UTF8-CONT})
UTF8-CONT  [\x80-\xbf]
%%
{LWS} ;
{SUBJECTS} 	{
		PASSCHAR('s')
		return SUBJECT;
	}
{SUBJECTF} 	{
		SIPDEBUG("SIP PARSER LEXER UTF8 subject\n");
		PASSCHAR('f')
		return SUBJECT;
	}
{ORGANIZATION}	{
		SIPDEBUG("SIP PARSER LEXER UTF8 organization\n");
		return ORGANIZATION;
	}
{COLON}		{
		SIPDEBUG("SIP PARSER LEXER UTF8 colon\n");
		PASSTOKEN(yytext)
	 	return COLON;
	}
{TEXTUTF8NOCOLON} {
		SIPDEBUG("SIP PARSER LEXER UTF8 textutf8nocolon\n");
		PASSTOKEN(yytext)
		return TEXTUTF8NOCOLON;
	}
.|\n {
		return DEFAULT;
	}
%%
int yywrap(void)
{
 return 1;
}

void sip_lex_Utf8_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Utf8_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexUtf8Mutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserUtf8JumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Utf8_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexUtf8Mutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Utf8_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexUtf8Mutex);
#endif
}

int glbSipParserUtf8lex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserUtf8lexreal(yylval));
#else
	if (setjmp(glbSipParserUtf8JumpBuf) == 0)
	{
		return(glbSipParserUtf8lexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexUtf8Mutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Utf8_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserUtf8JumpBuf,1);
}
#endif

