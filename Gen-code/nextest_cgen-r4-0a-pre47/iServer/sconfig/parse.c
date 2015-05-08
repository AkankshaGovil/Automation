#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include "sconfig.h"

extern FILE *sconfigin;
extern int sconfiglineno;

//prototype for the parser function
int sconfigparse(void);

int
sconfig_parse_config(char *infile)
{
	int rc;

	if (!(sconfigin = fopen(infile, "r")))
	{
		printf("Could Not Open %s, errno %d\n", infile, errno);
		return 1;
	}

	rc = sconfigparse();

	fclose(sconfigin);

	fflush(stderr);

	return rc;
}

void sconfigerror(char *str)
{
	printf("parse_config:: Error in line no %d ", sconfiglineno);
	printf(str);
	printf("\n");
}

//#include "config.tab.c"
//#include "lex.sconfig.c"
