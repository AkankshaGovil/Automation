%option noyywrap

%{
#include <unistd.h>
#include <pthread.h>
#include "addrtype.h"

pthread_mutex_t addrTypeLexMutex = PTHREAD_MUTEX_INITIALIZER;

%}

%%

[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}   { return IPADDR; }

.*"."   { return ABS_DN; }

.*".".* { return DN; }

.*      { return HOSTNAME; }

[\n]	{ return -1; }

%%


int DetermineAddrType(char *host)
{
	int rc = -1;

	if(host)
	{
		pthread_mutex_lock(&addrTypeLexMutex);

		yy_scan_string(host);

		rc = yylex();

		yy_delete_buffer(YY_CURRENT_BUFFER);
		
		pthread_mutex_unlock(&addrTypeLexMutex);
	}

	return rc;
}
