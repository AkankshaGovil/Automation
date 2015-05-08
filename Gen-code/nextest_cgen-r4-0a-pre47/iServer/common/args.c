#include "common.h"

int
CountArgs(char *argsin)
{
	int nargs = 0;
	char *token = NULL;
        char *ptrptr; 
	do
	{
#ifdef SUNOS
		token = strtok_r(argsin, " \r\t\n\0", &ptrptr);
#else
		token = strsep(&argsin, " \r\t\n\0");
#endif
		if (token && strlen(token))
		{
			nargs++;
		}
	}
	while (token && strlen(token));

	return nargs;
}

int
ExtractArgs(char *argsin, int *argc, char **argv, int xargv)
{
	int nargs = 0;
	char *token = NULL;
	char *ptrptr;
	do
	{
#ifdef SUNOS
		token = strtok_r(argsin, " \r\t\n\0", &ptrptr);
#else
		token = strsep(&argsin, " \r\t\n\0");
#endif
		if (token && strlen(token))
		{
			argv[nargs++] = token;
		}
	}
	while (token && strlen(token) && (nargs <xargv));

	*argc = nargs;

	return nargs;
}

/* Will extract only the next argument, and return to the caller
 */
int
ExtractArg(char *argsin, char **arg, char **argsout)
{
	int argc;
	char *argv[1] = {0};

	if (ExtractArgs(argsin, &argc, argv, 1) > 0)
	{
		*arg = argv[0];
		*argsout = argv[0] + strlen(argv[0]) +1;
		return 1;
	}

	return -1;
}

