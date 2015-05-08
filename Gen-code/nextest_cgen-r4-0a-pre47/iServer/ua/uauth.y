%{
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "ipc.h"
#include "serverdb.h"
#include <malloc.h>
#include "serverp.h"
#include "ssip.h"

#ifdef YYSTYPE
#undef YYSTYPE
#endif 

typedef struct yy_buffer_state *YY_BUFFER_STATE;
int uauthlex(void);
void uautherror(char *);
YY_BUFFER_STATE uauth_scan_string(const char*);

pthread_mutex_t uauthMutex = PTHREAD_MUTEX_INITIALIZER;

char *authorization;
char *schema;
char *realm;
char *nonce;
char *qop;
char *stale;
char *opaque;
char *algorithm;

%}

%union {
    char    *string;     /* string buffer */
}

%token PROXY_AUTHENTICATE WWW_AUTHENTICATE
%token BASIC DIGEST
%token REALM NONCE QOP STALE OPAQUE ALGORITHM
%token MD5 FALSE TRUE
%token <string> QSTRING

%type <string> qstring

%%

authenticate: www_authenticate { authorization = "Authorization"; }
	| proxy_authenticate { authorization = "Proxy-Authorization"; }
	;

proxy_authenticate: PROXY_AUTHENTICATE basic_auth { schema = "Basic"; }
	| PROXY_AUTHENTICATE digest_auth { schema = "Digest"; }
	;

www_authenticate: WWW_AUTHENTICATE basic_auth { schema = "Basic"; }
	| WWW_AUTHENTICATE digest_auth { schema = "Digest"; }
	;

basic_auth: BASIC param_list
	;

digest_auth: DIGEST param_list
	;

param_list: param
	| param_list ',' param
	;

param: realm
	| nonce
	| opaque
	| qop
	| stale
	| algorithm
	;

realm: REALM '=' qstring { realm = $3; }
	;
	
nonce: NONCE '=' qstring { nonce = $3; }
	;

opaque: OPAQUE '=' qstring { opaque = $3; }
	;

qop: QOP '=' qstring { qop = $3; }
	;

stale: STALE '=' TRUE { stale = "true"; }
	| STALE '=' FALSE { stale = "false"; }
	;

algorithm: ALGORITHM '=' MD5 { algorithm = "md5"; }
	;

qstring: QSTRING { $$ = $1; }
	;

%%

#define SVal(s) (s ? s : "")
#define CheckFree(x) if(x) free(x)

char* createAuth(NetoidInfoEntry *infoEntry, char *method, char *authenticate)
{
	char *username, *digest_uri, *password;
	char A1[256], A2[256], auth[256];

	if(authenticate == NULL)
	{
		return NULL;
	}

	if(infoEntry == 0 || (infoEntry->stateFlags & CL_UAREG && strlen(infoEntry->passwd) == 0))
	{
		return NULL;
	}

	pthread_mutex_lock(&uauthMutex);

	authorization = schema = realm = nonce = qop = stale = opaque = algorithm = NULL;

	username = infoEntry->phone;
	digest_uri = infoEntry->uri;
	password = infoEntry->passwd;

	uauth_scan_string(authenticate);

	if(uauthparse() != 0)
	{
		uauth_release_string();

		pthread_mutex_unlock(&uauthMutex);
		return NULL;
	}

	uauth_release_string();

	if(strcmp(schema, "Digest") == 0)
	{
		snprintf(A1, sizeof(A1), "%s:%s:%s", SVal(username), SVal(realm), SVal(password));
		HashString(A1, A1);

		snprintf(A2, sizeof(A2), "%s:%s", SVal(method), SVal(digest_uri));
		HashString(A2, A2);

		snprintf(A1, sizeof(A1), "%s:%s:%s", A1, SVal(nonce), A2);
		HashString(A1, A1);
	}
	else
	{
		// Basic ???
	}

	snprintf(auth, sizeof(auth), "%s: %s username=\"%s\",realm=\"%s\",uri=\"%s\","
									"response=\"%s\",nonce=\"%s\",opaque=\"%s\",algorithm=md5",
									SVal(authorization), SVal(schema), SVal(username),
									SVal(realm), SVal(digest_uri), A1, SVal(nonce), SVal(opaque));

	pthread_mutex_unlock(&uauthMutex);

	CheckFree(realm);
	CheckFree(nonce);
	CheckFree(qop);
	CheckFree(opaque);

	return(strdup(auth));
}


void uautherror(char *string)
{
}

#if 0
int main(int argc, char **argv)
{
	int rc = -1;
	char *auth_string = "digest nonce=\"123\", realm=\"nextone.com\",algorithm=md5";

	++argv;

	yy_scan_string(*argv);

	rc = yyparse();

	if(rc == 0)
	{
		if(schema != 0)
			printf("%s\n", schema == 1 ? "Basic" : "Digest");
		if(realm_value) printf("realm: %s\n", realm_value);
		if(nonce_value) printf("nonce: %s\n", nonce_value);
		if(stale_value) printf("stale: %s\n", stale_value);
		if(opaque_value) printf("opaque: %s\n", opaque_value);
		if(algorithm_value) printf("algorithm: %s\n", algorithm_value);
	}
	else
	{
		printf("invalid auth string\n");
	}
}
#endif
