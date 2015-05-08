// simulate the behavior of the jServer
#include "cli.h"

#define CMDLEN	1024

int
main(int argc, char **argv)
{
	FILE *input = NULL;
	char cmd[CMDLEN];

	if (argc < 2)
	{
		fprintf(stderr, "%s no file name specified\n", argv[0]);
		return -1;
	}
	
	if (!(input = fopen(argv[1], "r")))
	{
		fprintf(stderr, "%s cannot open file %s\n", argv[0], argv[1]);
		return -1;
	}

	CliCmdHistoryInit();

	cliLibFlags = COMMANDF_ALL;

	if (SHM_Init(ISERVER_CACHE_INDEX) < 0)
	{
		printf("SHM_Init failed\n");
		exit(-1);
	}

	while (fgets(cmd, CMDLEN, input))
	{
		fprintf(stdout, "executing %s\n", cmd);
		ExecuteCliCommand(cmd);
	}
}

int
ExecuteCliCommand(char *cmd)
{
	int argc;
	char *argv[254];
	int ret;

	ExtractArgs(cmd, &argc, argv, 254);
	
	if (!strncmp(argv[0]+strlen(argv[0])-3, "cli", 3))
	{
		//skip this
		ret = ProcessCommand(argc-1, argv+1);
	}
	else
	{
		ret = ProcessCommand(argc, argv);
	}

	fprintf(stdout, "%s\n", (ret==xleOk)?"success":"error");

	return ret;
}
