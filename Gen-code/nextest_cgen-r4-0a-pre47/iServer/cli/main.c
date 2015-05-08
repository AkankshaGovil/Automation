#include <malloc.h>
#include <stdlib.h>
#include "common.h"
#include "cli.h"
#include "serverp.h"

//char	config_file[60] = CONFIG_FILENAME;

//char    pidfile[256];
unsigned long histid1, histid2, orig_size, current_size;
unsigned int mark;

int
main( int argc, char **argv )
{
    char *denv;
    sigset_t new, old;
	int c;
	int rc;

    denv = (char *)getenv ("ALOID_DEBUG");

	// Set up function pointers for main
	CliSetupExtFns();

	NetLogInit();
	NetSyslogOpen("cli", NETLOG_ASYNC);

	cliLibFlags = COMMANDF_ALL;
	cli_ix = 0;

	progName = argv[0];

	CliCmdHistoryInit();

	while((c = getopt(argc, argv, "oxvrisd:")) != EOF)
	{
		switch(c)
		{
		case 'v':
			/* Give version information */
			printf ("%s %s.%s, %s\n%s\n\n",
					CLI_NAME,
					CLI_VERSION,
					CLI_MINOR,
					CLI_BUILDDATE,
					CLI_COPYRIGHT);

			exit(0);
			break;

		case 'r':
			send2Rs = 0;
#ifndef SAVE_SEC
			saveindb = 0;
#endif
			break;

		case 's':
			saveindb = 0;
			break;

		case 'x':
			cli_debug = 1;
			break;

		case 'i':
			cli_ix = 1;
			break;

		case 'd':
			strncpy(clifiledir, optarg, sizeof(clifiledir));
			break;

		case 'o':
			(void) freopen("/dev/null", "a", stdout);
			break;

		case '?':
			exit(-1);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (SHM_Init(ISERVER_CACHE_INDEX) < 0)
	{
		printf("SHM_Init failed\n");
		NETERROR(MCLI, ("SHM_Init failed\n"));
		exit(-1);
	}

	// Block ALL signals here
	FillAllSignals(&new);
	BlockAllSignals(&new, &old);

#ifdef _DMALLOC_
#ifdef NETOID_LINUX
	{
		extern char *dmalloc_logpath;
		dmalloc_logpath = "./malloc.inuse";
	}
#else
	{
		extern char *dmalloc_logpath;
		dmalloc_logpath = "./cli_malloc.inuse";
	}
#endif

#elif _DBMALLOC_ 

	//extern int malloc_preamble;
	{
		union dbmalloptarg val;

		val.i = 1;
		val.i = 0;
		dbmallopt(MALLOC_CKCHAIN, &val);

		val.i = 1;
		dbmallopt(MALLOC_SHOWLINKS, &val);

		val.i = 1;
		dbmallopt(MALLOC_DETAIL, &val);

		val.i = 129;
		val.i = 0;
		dbmallopt(MALLOC_FATAL, &val);

		val.str = "malloc.errs";
		//dbmallopt(MALLOC_ERRFILE, &val);

		val.i = 0;
		dbmallopt(MALLOC_REUSE, &val);

		val.i = 0;
		dbmallopt(MALLOC_CKDATA, &val);
	}
	
#endif

#ifdef _DMALLOC_
		dmalloc_message("starting new log");
		mark = dmalloc_mark();
#elif _DBMALLOC_ 
		orig_size = malloc_inuse(&histid1);
#endif

    rc = CliMain(argc, argv);
	
#ifdef _DMALLOC_
		dmalloc_log_changed(mark, 1, 0, 1);
		dmalloc_message("end of log");
#elif _DBMALLOC_ 
		current_size = malloc_inuse(&histid2);

		if(current_size != orig_size)
		{
			int fd;
			fd = open("malloc.inuse", O_CREAT|O_RDWR|O_TRUNC);
			malloc_list(fd, histid1, histid2);
			close(fd);
		}
#endif

    UnblockAllSignals(&old, 0);

    exit (rc);
}

