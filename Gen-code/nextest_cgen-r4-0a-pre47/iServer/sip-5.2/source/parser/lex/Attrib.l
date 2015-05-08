%{
/***********************************************************************
 ** FUNCTION:
 **             Lex file for Attrib headers

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
 ** 24/11/99  Ashok Roy			         Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *********************************************************************/
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "siplist.h"
#include "Attrib.tab.h"
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Attrib_fatal_error(msg)
static jmp_buf glbSipParserAttribJumpBuf;
void sip_lex_Attrib_fatal_error(const char *pErrMsg); 
#endif
#define yylval glbSipParserAttriblval
#define YY_DECL int glbSipParserAttriblexreal(YYSTYPE *lval)
%}

LWS [ \t]*
CR [\n\r]+
AEQ "a"{LWS}"="

%%
[ \t]+ ;
"," ;
":"     {
			return COLON;
		}
{AEQ}	{ 
			return AEQ; 
		}
([^\n\r=:]+)	{
		PASSTOKEN(yytext)
		return(BYTESTRNOCOLON);
	}
":"([^\n\r]+)      {
		PASSTOKEN(&(yytext[1]))
		return(BYTESTR);
	}
{CR} {return(CR);}
.|\n {
		return DEFAULT;
	}
%%
int yywrap(void )
{
	return(1);
}
void sip_lex_Attrib_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}


int sip_lex_Attrib_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexAttribMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserAttribJumpBuf) == 0)
	{
		
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Attrib_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexAttribMutex);
#endif
		return 1;
	}
#endif	
}



void sip_lex_Attrib_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexAttribMutex);
#endif
}

int glbSipParserAttriblex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserAttriblexreal(yylval));
#else
	if (setjmp(glbSipParserAttribJumpBuf) == 0)
	{
		return(glbSipParserAttriblexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexAttribMutex);
#endif
		return 0;
	}
#endif

}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Attrib_fatal_error(const char* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,(SIP_S8bit *)pMessage);	
	longjmp(glbSipParserAttribJumpBuf,1);
}
#endif

