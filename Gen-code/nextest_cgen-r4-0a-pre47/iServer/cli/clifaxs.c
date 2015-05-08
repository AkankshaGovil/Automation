#include "cli.h"
#include "serverp.h"
#include "gw.h"
#include "lsconfig.h"
#include "log.h"


static char s1[30];
#if 0
int myConfigServerType = CONFIG_SERPLEX_FAXS;
#endif

/* Default value for config_file */
char config_file[60] = CONFIG_FILENAME;

char    pidfile[256];
struct ifi_info *ifihead;
char *getenv(const char *name);

int
HandleFaxsLkup(Command *comm, int argc, char **argv)
{
     char *faxId;
     FaxEntry *faxEntry, *lastFaxEntry;

     log(LOG_DEBUG, 0, "Entering Faxs Lkup with argc=%d\n", argc);
     log(LOG_DEBUG, 0, "Looking for Faxes for Phone No. %s\n", argv[0]);

     if (argc != 1)
     {
	  /* Here we prompt the user for the rest of the 
	   * information
	   */
	  HandleCommandUsage(comm, argc, argv);
	  return -xleInsuffArgs;
     }

     if (OpenDatabases((DefCommandData *)comm->data) < 0)
     {
		return -xleOpNoPerm;
     }

     for (faxEntry = DbGetFirstFaxEntry(GDBMF(comm->data, DB_eFax)); 
			faxEntry != 0; 
	  faxEntry = DbGetNextFaxEntry(GDBMF(comm->data, DB_eFax), faxId, sizeof(FaxKey)), 
	  free(lastFaxEntry))
     {
	  if (strcmp(faxEntry->fax_phone_number, argv[0]) == 0)
	  {
	  CLIPRINTF((stdout, "\n\tSource e-mail:\t\t%s\n", faxEntry->src_email_addr));
	  CLIPRINTF((stdout, "\tPhone No:\t\t%s\n", faxEntry->fax_phone_number));
	  CLIPRINTF((stdout, "\tPhone No Type:\t\t%s\n", 
	  				(faxEntry->fax_phone_numtype == 0x1) ? "LUS" : "VPNS"));
	  CLIPRINTF((stdout, "\tDestination e-mail:\t%s\n", faxEntry->dest_email_addr));
	  CLIPRINTF((stdout, "\tFile name:\t\t%s\n", faxEntry->dest_file_name));
	  CLIPRINTF((stdout, "\tRetry Count:\t\t%d\n", faxEntry->fax_retry_count));
	  }

	  faxId = faxEntry->dest_file_name;
	  lastFaxEntry = faxEntry;
     }

	CloseDatabases((DefCommandData *)comm->data);

     return xleOk;
}

int
HandleFaxsDelete(Command *comm, int argc, char **argv)
{
	char tmp_arr[256];
	char * ptr;
     char fn[] = "HandleFaxsDelete():";
     FaxKey faxKey;
	void *addr;

     if (argc != 1)
     {
	  /* Here we prompt the user for the rest of the 
	   * information
	   */
	  HandleCommandUsage(comm, argc, argv);
	  return -xleInsuffArgs;
     }

     memset(&faxKey, 0, sizeof(FaxKey));
     strcpy(faxKey.dest_file_name, argv[0]);

     log(LOG_DEBUG, 0, "Fax File Name is %s\n", argv[0]);
     
	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}
	
     if (DbDeleteFaxEntry(GDBMF(comm->data, DB_eFax), &faxKey, sizeof(FaxKey)) < 0)
     {
	  log(LOG_ERR, errno, "database delete error\n");
     }
     else
     {
	  CLIPRINTF((stdout, "Entry Deleted from Database Successfully\n"));
     }
	CloseDatabases((DefCommandData *)comm->data);
     

   /*
      Set the config file path if SERPLEXPATH is defined.
   */
   setConfigFile();
#if 0
	DoConfig();
#endif
	memset (tmp_arr, 0, sizeof(tmp_arr));
	ptr = tmp_arr;
	strcpy(ptr, fax_dir_pathname);
	ptr += strlen(fax_dir_pathname);
	strcpy(ptr, "/");
	ptr++;
	strcpy(ptr, argv[0]);
	if (unlink(tmp_arr) < 0)
	{
		CLIPRINTF((stdout, "Error Deleting File [%s]\n", tmp_arr));
	}
	else
	{
		CLIPRINTF((stdout, "File [%s] Deleted successfully.\n", tmp_arr));
	}

     return xleOk;
}

int
HandleFaxsAdd(Command *comm, int argc, char **argv)
{
     char fn[] = "HandleFaxsAdd():";
     FaxKey faxKey;
     FaxEntry *_faxentry;
	FaxEntry FaxDbEntry;

     if (argc != 3)
     {
	  /* Here we prompt the user for the rest of the 
	   * information
	   */
	  HandleCommandUsage(comm, argc, argv);
	  return -xleInsuffArgs;
     }

     log(LOG_DEBUG, 0, "Fax Phone Num Type is %s\n", argv[0]);
     log(LOG_DEBUG, 0, "Fax Phone Num is %s\n", argv[1]);
     log(LOG_DEBUG, 0, "Fax File Name is %s\n", argv[2]);

     memset(&faxKey, 0, sizeof(FaxKey));
     memset(&FaxDbEntry, 0, sizeof(FaxEntry));
     strcpy(faxKey.dest_file_name, argv[2]);
     strcpy(FaxDbEntry.dest_file_name, argv[2]);
	 strcpy(FaxDbEntry.fax_phone_number, argv[1]);
	FaxDbEntry.fax_retry_count = 0;
	FaxDbEntry.fax_phone_numtype = ((strcmp(argv[0], "LUS") == 0) ? LUS_TYPE:VPNS_TYPE);

	if (OpenDatabases((DefCommandData *)comm->data) < 0)
	{
		return -xleOpNoPerm;
	}
	
	_faxentry = DbFindFaxEntry(GDBMF(comm->data, DB_eFax), (char *)&faxKey, sizeof(FaxKey));

	if (_faxentry == (FaxEntry *) 0)
	{
		if (DbStoreFaxEntry(GDBMF(comm->data, DB_eFax), &FaxDbEntry, &faxKey, sizeof(FaxKey)) < 0)
		{
			ERROR(MDB, ("HandleFaxsAdd: Entry [%s] [%s] failed!\n", argv[1], argv[2]));
		}
		else
		{
			DEBUG(MFAXP, NETLOG_DEBUG4, ("HandleFaxsAdd: Entry [%s] [%s] added successfully!\n", 
					argv[1], argv[2]));
		}

	}
	else
	{
		ERROR(MDB, ("HandleFaxsAdd: Entry [%s] [%s] already exists!\n", argv[1], argv[2]));
		free(_faxentry);
	}

     
	CloseDatabases((DefCommandData *)comm->data);
     return xleOk;
}

int
HandleFaxsList(Command *comm, int argc, char **argv)
{
     char *faxId;
     FaxEntry *faxEntry, *lastFaxEntry;

     log(LOG_DEBUG, 0, "Entering Faxs List with argc=%d\n", argc);

     if (argc != 0)
     {
	  /* Here we prompt the user for the rest of the 
	   * information
	   */
	  HandleCommandUsage(comm, argc, argv);
	  return -xleInsuffArgs;
     }

     if (OpenDatabases((DefCommandData *)comm->data) < 0)
     {
		return -xleOpNoPerm;
     }

     for (faxEntry = DbGetFirstFaxEntry(GDBMF(comm->data, DB_eFax)); 
			faxEntry != 0; 
	  faxEntry = DbGetNextFaxEntry(GDBMF(comm->data, DB_eFax), faxId, sizeof(FaxKey)), 
	  free(lastFaxEntry))
     {
	  CLIPRINTF((stdout, "\n\tSource e-mail:\t\t%s\n", faxEntry->src_email_addr));
	  CLIPRINTF((stdout, "\tPhone No:\t\t%s\n", faxEntry->fax_phone_number));
	  CLIPRINTF((stdout, "\tPhone No Type:\t\t%s\n", 
	  				(faxEntry->fax_phone_numtype == 0x1) ? "LUS" : "VPNS"));
	  CLIPRINTF((stdout, "\tDestination e-mail:\t%s\n", faxEntry->dest_email_addr));
	  CLIPRINTF((stdout, "\tFile name:\t\t%s\n", faxEntry->dest_file_name));
	  CLIPRINTF((stdout, "\tRetry Count:\t\t%d\n", faxEntry->fax_retry_count));

	  faxId = faxEntry->dest_file_name;
	  lastFaxEntry = faxEntry;
     }

	CloseDatabases((DefCommandData *)comm->data);

     return xleOk;
}

#if 0
int
ProcessConfig(void)
{
	int match = -1;

	/* Process Configuration, read from the config file */
	match = FindServerConfig();

	if (match == -1)
	{
		fprintf(stdout, "Not Configured to run...\n");
		exit(0);
	}

}
#endif
