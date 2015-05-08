#ifndef 	__dbrep_hist_h
#define 	__dbrep_hist_h

#include 	<gdbm.h>
#include 	"ipckey.h"

#define		HISTDB_LOCK_FILE	( DBLOCKDIR	"/histdb.lock" )
#define		SEQNO_LOCK_FILE		( DBLOCKDIR "/seqno.lock" )
#define		DBREV_LOCK_FILE		( DBLOCKDIR "/dbrev.lock" )

/* Maximum Size of the cli history */
#define 	CLI_MAX_HIST		2000 		/* WARNING: Keep same as rsd_common.h */
extern int  cli_max_hist;

#define		CLI_HIST_DNAME		"/usr/local/nextone/databases" /* Directory name */
#define		CLI_HIST_FNAME		"cli_hist.gdbm"
#define		CLI_HIST_FILE		"/usr/local/nextone/databases/cli_hist.gdbm"
#define		CLI_SEQNUM_FNAME	".cli_seqno"	
#define		CLI_SEQNUM_FILE		"/usr/local/nextone/databases/.cli_seqno"
#define		CLI_DBREV_FNAME		".dbrev"	
#define		CLI_DBREV_FILE		"/usr/local/nextone/databases/.dbrev"
#define		CLI_NULL_SEQNO		((long)(-1))
#define     IS_NULL_SEQNO(seqno)	((seqno) == CLI_NULL_SEQNO)
#define 	CLI_CUR_IND_KEY		"indexkey"

/* Some Macros to change the current index i by j, in a circular array */
#define		cyc_inc(i,j)		(((i) + (j)) % MAX_HIST_SIZE)	
#define		cyc_dec(i,j)		(((i) - (j)) % MAX_HIST_SIZE)	

/* History Database Related functions */
typedef struct {
	long	seqno;
	char 	*clicmd;   /* Points to a Cmd buffer */
	int 	cmdlen;
} CliEntry;

typedef struct { 
GDBM_FILE	gdbmf; 
int			fd; 
} HDB; 

/*
 * Public Functions 
 */

HDB*
OpenCliHist(void);   /* Open and lock the History database */

void
CloseCliHist(HDB *histdb); 	/* Close and unlock the History database */

int
StoreCliEntry(HDB *hdbp, CliEntry *clip); /* Store Cli entry
	in the opened database */

CliEntry *
FindCliEntry(HDB *hdbp, long seqno);  /* Find cli cmd corresponding
	to the seqno */

CliEntry **
FindCliEntries(HDB *hdbp, long seqno); /* Find cli cmd entries
	since seqno (inclusive) */

void
PrintCliHist(HDB *hdbp);

void
FreeCliEntry(CliEntry *clip);

void
FreeCliEntries(CliEntry **clipp);

long
FindCurInd(GDBM_FILE dbf);  /* Gets the Current index from the Cli History
	database. This function is antiquated and GetSeqNo(SEQ_RD) should be
	preferred in its place. */

long
GetSeqNum(int fd, int operation);	/* Returns the available sequence
	number and may increment the sequence number counter or may not
	depending on the operation*/

/* Sequence Number related Stuff */
#define SEQ_RD	0
#define SEQ_INC	1

long GetCliSeqNum(void); 	/* Returns the next available sequence number */
long ReadSeqNum(void); 		/* Returns the last used sequence number */

int
LockSeqNum(void);   		/* Open and lock the sequence number file. 
						       Return the fd */

void
UnlockSeqNum(int fd); 		/* Unlock and close the file */


/* Database Revision Number related stuff */

#define	DBREV_RD	0		/* Read the current revision number */
#define	DBREV_INC	1		/* Increment revision number by 1 */
#define	DBREV_MOD	2 	  	/* Modify revsion number to the argument */

int ModDBRevNum(int newrev); /* Modify the revision number to the specified */
int GetNextDBRevNum(void); 
int ReadDBRevNum(void);

int GetDBRevNum(int fd, int operation, void *arg); 	
/* Returns the db revision number based on the operation and argument */

int
LockDBRevNum(void);			/* Open and lock the db revision file. Return the fd */

void
UnlockDBRevNum(int fd); 	/* Unlock and close the file */

/* Private functions */
void InitCliHist(GDBM_FILE dbf);
void gdbm_fatal(char *str);
char **FindEntries(GDBM_FILE dbf, long seqno);
int FindEntry(GDBM_FILE dbf, long seqno, char **entry, int *entrylenp);
int StoreEntry(GDBM_FILE dbf, long seqno, const char *cmdp, int cmdlen);
int StoreCurInd(GDBM_FILE dbf, long cur_ind);
int CmdCompare(const void *d1, const void *d2);
void Chkfree (void *ptr);

#endif 	/* __dbrep_hist_h */
