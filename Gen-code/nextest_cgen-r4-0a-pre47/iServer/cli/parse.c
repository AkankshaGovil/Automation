#include <stdio.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "cli.h"

extern FILE *yyin;
extern int yylineno;
extern int yyparse(void);

//#define YYDEBUG	1
// 0 for success, non zero (<0 or >0) for error
// <0 means our error
// >0 means parse errors, which go unhandled

int
parse_input(char *infile)
{
	int rc = 0;
	extern int yydebug;

	if (!(yyin = fopen(infile, "r")))
	{
		fprintf(stdout, "Could not open %s\n",
				infile);
		return -1;
	}
#if YYDEBUG
	yydebug = 1;
#endif

	rc = yyparse();

	fclose(yyin);

	CLIPRINTF((stdout, "\n"));

	return rc;
}

int
yyerror(char *str)
{
	printf("\nParse Error at line no %d \n", yylineno);
	ResetDatabaseParsing();

	exit(-1);
}

void
print_progress(void)
{
	struct stat buf;
	double x, y, z;
	int p;

	fstat(fileno(yyin), &buf);
	if (cliLibFlags)
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

	x = ftell(yyin);
	y = buf.st_size;
	z = x/y;
	z *= 100;
	p = z;
	if (p <= 0 ) return;
	if (p > 100) p = 100;
	if (cliLibFlags)
		printf("%3.3d%% of db processed", p);
}

#include "one.tab.c"
#include "lex.yy.c"
