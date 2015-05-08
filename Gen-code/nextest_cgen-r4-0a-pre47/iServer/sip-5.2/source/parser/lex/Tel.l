/***********************************************************************
 ** FUNCTION:
 **             Yacc file for Tel Url

 *********************************************************************
 **
 ** FILENAME:
 ** Tel.l
 **
 ** DESCRIPTION:
 ** 
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 5-1-2001   	Mahesh Govind		Initial creation			Support For Tel Url
 ** Copyright 2001, Hughes Software Systems, Ltd.
 *********************************************************************/
%{
#include "portlayer.h"
#include "sipparserinc.h"
#include "sipdecodeintrnl.h"
#include "teldecodeintrnl.h"
#include "Tel.tab.h"
#define yylval glbSipParserTellval
#include "siplist.h"
 int glbtelabort=0;
 int glbtelabort1=0;
 int SetState=0;
#define YY_DECL int glbSipParserTellexreal(YYSTYPE *lval)
#ifndef SIP_NOFLEXRECOVERY
#include <setjmp.h>
#define YY_FATAL_ERROR(msg) sip_lex_Tel_fatal_error(msg)
static jmp_buf glbSipParserTelJumpBuf;
void sip_lex_Tel_fatal_error(const SIP_S8bit *pErrMsg); 
#endif
%}
%s AllowQstring
%s AllowEquals
%s AllowEqualsTag
%s TokenPlus
UALPHA [E-Z]
PAUSETOKEN ("p"|"w"|"P"|"W")
LALPHA [a-o]|[q-v]|[x-z]
DTMFALPHA [A-D]
ALPHA ({LALPHA}|{UALPHA}|{DTMFALPHA}|{PAUSETOKEN})
HEX [0-9]|[A-F]|[a-f]
HEXTOKEN {HEX}+
DIGIT [0-9]
DIGITS {DIGIT}+
HEX4 {HEX}{1,4}
HEXSEQ {HEX4}((":"{HEX4})*)
HEXPART ({HEXSEQ}|{HEXSEQ}"::"|{HEXSEQ}"::"{HEXSEQ}|"::"{HEXSEQ})
IPV6ADDR "["{LWS}({HEXPART}|{HEXPART}":"{DIGITS}"."{DIGITS}"."{DIGITS}"."{DIGITS}){LWS}"]"
ALPHANUM ({LALPHA}|{UALPHA}|{DTMFALPHA}|{PAUSETOKEN}|{DIGIT})+
LWS [ \t]*
PLUS  	("+")
DTMFTOKEN ({DTMFALPHA}|"*")
TOKENSEPARATOR ("-"|".")
TOKEN ({LALPHA}|{UALPHA}|{DIGIT}|{TOKENSEPARATOR}|{PAUSETOKEN}|{DTMFTOKEN}|"!"|"%25"|"_"|"~"|"'"|"%60")+
TOKENPLUS ({TOKEN}|"%2b"|"%2B"|"+"|"("|")")+
EQUALS "="
MARK ("-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")")
LANGLE {LWS}*"<"
RANGLE ">"
TELCOLON [Tt][Ee][Ll]{LWS}":"
TOKENEQUALS {TOKEN}{LWS}"="
SEMTOKEN ";"{LWS}{TOKEN}
QSTRING "%22"(([^"\\\n])|("%5"[cC]"%22".))*"%22"
VISUALSEPARATOR ({TOKENSEPARATOR}|"("|")")
PHONEDIGIT ({DIGIT}|{VISUALSEPARATOR})+
DTMFDIGIT ({DTMFTOKEN}|"%23")+
PHONENUMBER ({PHONEDIGIT}|{DTMFDIGIT}|{PAUSETOKEN})+
GENERALTOKEN ({ALPHA}|{DIGIT}|{TOKENSEPARATOR}|{DTMFDIGIT}|"~"|"%24"|"%5e"|"%5E"|"'"|"%60"|"!"|"%25"|"%7c"|"%7C"|"%26"|"_"|"&"|"$")+
GENERALTOKENS ({GENERALTOKEN})+
SEMICOLONGENERALTOKEN ";"{LWS}({GENERALTOKEN}|"%2b"|"%2B"|"+")+
QUESTIONMARKGENERALTOKEN "?"{LWS}("%2b"|"%2B"|"+"|{GENERALTOKEN})+
EQUALSGENERALTOKEN "="{LWS}("%2b"|"%2B"|"+"|{GENERALTOKEN})+
EQUALSTAGGENERALTOKEN "="{LWS}("%2b"|"%2B"|{GENERALTOKEN}|"("|")"|"%22"|"%40"|"%2c"|"%2C"|"%7b"|"%7B"|"%7d"|"%7D"|"%2f"|"%2F"|"%5c"|"%5C"|"%5b"|"%5B"|"%5d"|"%5D"|"%3a"|"%3A"|"%3c"|"%3C"|"%3d"|"%3D"|"%3E"|"%3e"|"%3f"|"%3F"|"\/"|"?"|":"|"@"|"&"|"+"|"$"|","|"["|"]")+
ISDNTAG ";"{LWS}[Ii][Ss][uU][Bb]{LWS}
POSTDIALTAG ";"{LWS}[pP][Oo][Ss][Tt][Dd]{LWS}
PHONETAG ";"{LWS}[pP][hH][oO][Nn][Ee]"-"[Cc][Oo][Nn][Tt][Ee][Xx][tT]
PROVIDERTAG ";"{LWS}[Tt][Ss][Pp]
%%
{LWS} ;
{TELCOLON} {
		SIPDEBUG("SIP PARSER LEXER TELCOLON:\n");
		return TELCOLON;
	}
{PLUS}  {SIPDEBUG("SIP PARSER LEXER TEL PLUS rule\n");
		return PLUS;
		}
{ISDNTAG}  {
		SIPDEBUG("SIP PARSER LEXER TEL ISDNTAG rule\n");
		return ISDNTAG;
		}
{POSTDIALTAG}  {
		SIPDEBUG("SIP PARSER LEXER TEL POSTDIALTAG rule\n");
		return POSTDIALTAG;
		}
{PHONETAG}  {
		SIPDEBUG("SIP PARSER LEXER TEL PHONETAG rule\n");
		BEGIN AllowEqualsTag;
		return PHONETAG;
		}
{PROVIDERTAG}  {
		SIPDEBUG("SIP PARSER LEXER TEL PROVIDERTAG rule\n");
		PASSTOKEN(yytext);
		BEGIN TokenPlus;
		return PROVIDERTAG;
		}
{DIGIT} 	{
		SIPDEBUG("SIP PARSER LEXER digit\n");
		PASSTOKEN(yytext);
		return DIGIT;
		}
{DTMFALPHA} 	{
		SIPDEBUG("SIP PARSER DTMFALPHA digit\n");
		PASSTOKEN(yytext);
		return DTMFALPHA;
		}

{PAUSETOKEN}  {SIPDEBUG("SIP PARSER LEXER TEL Pause\n");
		PASSTOKEN(yytext);
		return PAUSETOKEN;
		}
{EQUALS}	{
	  	SIPDEBUG("SIP PARSER LEXER TEL equals\n");
		if(glbtelabort==1)
		BEGIN AllowQstring;
		return EQUALS;
	}
{DTMFTOKEN}	{
			 
				SIPDEBUG("SIP PARSER LEXER TEL DTMF TOKEN rule\n");
				PASSTOKEN(yytext);
				return DTMFTOKEN;
			}


{TOKENSEPARATOR}	{
			 
				SIPDEBUG("SIP PARSER LEXER TEL TOKENSEPARATOR rule\n");
				PASSTOKEN(yytext);
				return TOKENSEPARATOR;
			}

{VISUALSEPARATOR}		{
			 
				SIPDEBUG("SIP PARSER LEXER TEL VISUALSEPARATOR rule\n");
				PASSTOKEN(yytext);
				return VISUALSEPARATOR;
			}

{PHONEDIGIT}	 	{
			 
				SIPDEBUG("SIP PARSER LEXER TEL PHONE DIGIT rule\n");
				PASSTOKEN(yytext);
				return PHONEDIGIT;
			}
{DTMFDIGIT}		{
		
					SIPDEBUG("SIP PARSER LEXER TEL DTMF DIGIT rule\n");
					PASSTOKEN(yytext);
					return DTMFDIGIT;
				}
{PHONENUMBER}		{
		
				SIPDEBUG("SIP PARSER LEXER TEL PHONE NUMBER rule\n");
					PASSTOKEN(yytext);
					return PHONENUMBER;
				}

{TOKEN}		{	
		
				SIPDEBUG("SIP PARSER LEXER TEL TOKEN rule\n");
				PASSTOKEN(yytext);
				return TOKEN;
				}
		

<TokenPlus>{TOKENPLUS}	{	
		
				SIPDEBUG("SIP PARSER LEXER TEL TOKENPLUS rule\n");
				PASSTOKEN(yytext);
				BEGIN 0;
				return TOKENPLUS;
				}

<AllowQstring>{QSTRING}		{	
		
				SIPDEBUG("SIP PARSER LEXER TEL QSTRING rule\n");
				PASSTOKEN(yytext);
				BEGIN 0;
				return QSTRING;
				}
	


{SEMICOLONGENERALTOKEN}	{	
		
				SIPDEBUG("SIP PARSER LEXER TEL SEMICOLONGENERALTOKEN rule\n");
				PASSTOKEN(yytext);
				 BEGIN AllowEquals;
				return SEMICOLONGENERALTOKEN;
				}
{QUESTIONMARKGENERALTOKEN}		{	
		
				SIPDEBUG("SIP PARSER LEXER TEL QUESTION GENERALTOKEN rule\n");
				PASSTOKEN(yytext);
				return QUESTIONMARKGENERALTOKEN;
				}
<AllowEquals>{EQUALSGENERALTOKEN}		{	
		
				SIPDEBUG("SIP PARSER LEXER TEL EQUALSGENERALTOKEN rule\n");
				PASSTOKEN(yytext);
				BEGIN 0;
				return EQUALSGENERALTOKEN;
				}
<AllowEqualsTag>{EQUALSTAGGENERALTOKEN}		{	
		
				SIPDEBUG("SIP PARSER LEXER TEL EQUALS TAG GENERALTOKEN rule\n");
				PASSTOKEN(yytext);
				BEGIN 0;
				return EQUALSTAGGENERALTOKEN;
				}
{IPV6ADDR} {
		SIPDEBUG("SIP PARSER LEXER TEL IPV6ADDR \n");
		PASSTOKEN(yytext);
		return  IPV6ADDR;
		}
.|\n	{
		
		SIPDEBUG("SIP PARSER LEXER TEL default rule\n");
		PASSTOKEN(yytext);
		return DEFAULT;
		}
%%
int yywrap()
{
 return 1;}
void sip_lex_Tel_free(void )
{
	yy_delete_buffer(yy_current_buffer);
}
void sip_lex_Tel_reset_state( void )
{
	BEGIN 0;
}

int sip_lex_Tel_scan_buffer(SIP_S8bit *pBuffer, SIP_U32bit dLength)
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLexTelMutex, FAST_MXLOCK_EXCLUSIVE );
#endif

#ifdef SIP_NOFLEXRECOVERY	
	yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
	return 0;
#else
	if(setjmp(glbSipParserTelJumpBuf) == 0)
	{
		yy_scan_buffer((char *) pBuffer, (yy_size_t) dLength);
		return 0;
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: sip_lex_Tel_scan_buffer"\
			"Got Flex fatal error");
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLexTelMutex);
#endif
		return 1;
	}
#endif	
}


void sip_lex_Tel_release_buffer(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTelMutex);
#endif
}


int glbSipParserTellex(YYSTYPE *yylval)
{
#ifdef SIP_NOFLEXRECOVERY
	return(glbSipParserTellexreal(yylval));
#else
	if (setjmp(glbSipParserTelJumpBuf) == 0)
	{
		return(glbSipParserTellexreal(yylval));
	}
	else
	{
		sip_error(SIP_Major,"SIP_ERROR: GOT FLEX FATAL ERROR while parsing \n");
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLexTelMutex);
#endif
		return 0;
	}
#endif


}

#ifndef SIP_NOFLEXRECOVERY
void sip_lex_Tel_fatal_error(const SIP_S8bit* pMessage)
{
#ifndef SIP_ERROR
	SIP_S8bit *dummy;
	dummy = (SIP_S8bit *)pMessage;
#endif
	sip_error(SIP_Major,pMessage);	
	longjmp(glbSipParserTelJumpBuf,1);
}
#endif

