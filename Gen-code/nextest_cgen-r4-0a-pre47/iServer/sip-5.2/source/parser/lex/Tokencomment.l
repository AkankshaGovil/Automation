%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Token comment Headers

 *********************************************************************
 **
 ** FILENAME:
 ** tokensltoken.l
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
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "Tokencomment.tab.h"
#define yylval glbSipParserTokencommentlval
#include "siplist.h"
void tokencommentlexer(char *pText, YYSTYPE *lval);
#define YY_DECL int glbSipParserTokencommentlexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Tokencomment_fatal_error(msg)
static jmp_buf glbSipParserTokencommentJumpBuf;
void sip_lex_Tokencomment_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}

UALPHA [A-Z]
LALPHA [a-z]
ALPHA ({LALPHA}|{UALPHA})
DIGIT [0-9]
ALPHANUM ({ALPHA}|{DIGIT})
TOKEN ({ALPHANUM}|"-"|"."|"!"|"%"|"*"|"_"|"+"|"~"|"'"|"`")+
LWS [ \t]*
SLASH "/"
COMMENT "("((.|[ \t])*)")"
COMMENTLWS ({LWS}{COMMENT}{LWS})
USERAGENT [Uu][Ss][Ee][Rr]-[Aa][Gg][Ee][Nn][Tt]{LWS}":"
SERVER [Ss][Ee][Rr][Vv][Ee][Rr]{LWS}":" 
TOKENLWS {TOKEN}{LWS}
%%
{LWS}  ;
{USERAGENT}	{ 
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT useragent\n");
		return USERAGENT; 
	}
{SERVER}	{ 
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT server\n");
		return SERVER; 
	}
{SLASH}		{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT slash\n");
   		return SLASH;
	}
{COMMENT}	{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT Comment\n");
		tokencommentlexer(yytext, lval);
		return COMMENT;
	}
{COMMENTLWS}	{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT CommentLWS\n");
		tokencommentlexer(yytext, lval);
		return COMMENTLWS;
	}
{TOKEN} 	{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT Token\n");
		PASSTOKEN(yytext);
		return TOKEN;
	}
{TOKENLWS} 	{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT TOKENLWS\n");
		PASSTOKEN(yytext);
		return TOKENLWS;
	}
.|\n		{
		SIPDEBUG("SIP PARSER LEXER TOKENCOMMENT default\n");
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
void tokencommentlexer(char* mytext, YYSTYPE *lval)
{
	char* tmptext;
	int bracketcount = 1 ; /* yytext begins with a ( */
	int origlen;
	int ndx = 1;
	int upndx;

	tmptext = mytext;
	origlen = strlen(yytext);


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

	lval->pToken = mytext;
	lval->dLength = ndx;
}



void sip_lex_Tokencomment_free(void)
{
	yy_delete_buffer(yy_current_buffer);
}

int sip_lex_Tokencomment_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexTokencommentMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserTokencommentJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Tokencomment_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexTokencommentMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Tokencomment_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokencommentMutex);
#endif
}

int glbSipParserTokencommentlex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserTokencommentlexreal(yylval));
#else
	if (setjmp(glbSipParserTokencommentJumpBuf) == 0)
	{
		return(glbSipParserTokencommentlexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTokencommentMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Tokencomment_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserTokencommentJumpBuf,1);
}
#endif

