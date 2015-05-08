%{
#include <unistd.h>
#include <string.h>
#include "uauth.tab.h"

%}

%option noyywrap

ws	[ \t]+
qstring \"[^\"\n]*[\"\n]


%%

{ws}	 	{ ; }

[=,]		{ return yytext[0]; }

Proxy-Authenticate:	{ return PROXY_AUTHENTICATE; }

WWW-Authenticate:	{ return WWW_AUTHENTICATE; }

basic		{ return BASIC; }

digest		{ return DIGEST; }

realm		{ return REALM; }

nonce		{ return NONCE; }

qop			{ return QOP; }

stale		{ return STALE; }

opaque		{ return OPAQUE; }

algorithm	{ return ALGORITHM; }

true		{ return TRUE; }

false		{ return FALSE; }

md5			{ return MD5; }

{qstring}	{
				uauthlval.string = strdup(uauthtext + 1);
				if(uauthlval.string[uauthleng-2] == '"')
					uauthlval.string[uauthleng-2] = '\0';

				return QSTRING;
			}

%%

void uauth_release_string()
{
	uauth_delete_buffer(YY_CURRENT_BUFFER);
}
