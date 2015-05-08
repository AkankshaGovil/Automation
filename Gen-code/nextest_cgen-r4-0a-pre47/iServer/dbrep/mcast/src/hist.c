#include	"unp.h"
#include	"rs.h"
#include	"hist.h"
#include	"flock.h"

extern int histdb_size;

/*
 * Open and flock the History database.
 * Returns a pointer to the Database Desciptor on success
 * and NULL on failure
 */
HDB*
OpenCliHist(void)
{
	int			fd = -1;
	GDBM_FILE	dbf;
	HDB			*hdbp;
	char 		*histname = CLI_HIST_FILE; 
	
	hdbp = NULL;

	HDBGetLock(HDB_hist); 
	dbf = gdbm_open(histname, 0, GDBM_WRITER, S_IRWXU|S_IROTH, gdbm_fatal);

	/* If file does not exist create, and initialize it */
	if (dbf == NULL) {
		if (gdbm_errno == GDBM_FILE_OPEN_ERROR) {
			dbf = gdbm_open(histname, 0, GDBM_WRCREAT, S_IRWXU|S_IROTH, gdbm_fatal);
			InitCliHist(dbf);
		}
		else {
			gdbm_fatal("");
			HDBRelLock(HDB_hist);
			return(NULL);
		}
	}
	if (dbf !=NULL) {
		if ((hdbp = malloc(sizeof(HDB))) == NULL) {
			NETERROR(MRSD, ("OpenCliHist: malloc failed - %s\n", strerror(errno)));
			HDBRelLock(HDB_hist);
			return(NULL);
		}
		hdbp->gdbmf = dbf;
		hdbp->fd = fd;
 	}

	return(hdbp);
}

/*
 * Close and unlock the History database
 */
void
CloseCliHist(HDB *hdbp)
{
	if (hdbp == NULL)
		return;

	gdbm_close(hdbp->gdbmf);
//	if (close(hdbp->fd) != 0)
//		perror("CloseCliHist close failed: ");

	Chkfree(hdbp);
	HDBRelLock(HDB_hist);
	return;
}

/*
 * Initialize a recently opened History database
 */
void
InitCliHist(GDBM_FILE dbf)
{
	StoreCurInd(dbf, 0);	/* Initialize the current index of the database to 0 */
}

int
StoreCliEntry(HDB *hdbp, CliEntry *clip)
{
	int 	rval;

	rval = StoreEntry(hdbp->gdbmf, clip->seqno, clip->clicmd, clip->cmdlen);
	if (StoreCurInd(hdbp->gdbmf, clip->seqno) != 0)
		NETERROR(MRSD, ("StoreCliEntry: Error storing current index\n"));

	return(rval);
}

int
StoreEntry(GDBM_FILE dbf, long seqno, const char *cmdp, int cmdlen)
{
	datum 	key, data;
	long	index;

	index = seqno - histdb_size;

	key.dptr = (void *)&index;
	key.dsize = sizeof(index);

	if (gdbm_exists(dbf, key) && (gdbm_delete(dbf, key) != 0)) {
		NETERROR(MRSD, ("gdbm delete error with key %ld", seqno));
		gdbm_fatal("");
		return(-1);
	}

	key.dptr = (void *)&seqno;
	key.dsize = sizeof(seqno);

	data.dptr = (void *)cmdp;
	data.dsize = cmdlen;

	return(gdbm_store(dbf, key, data, GDBM_INSERT));
}

int
StoreCurInd(GDBM_FILE dbf, long cur_ind)
{
	datum 	key, data;

	key.dptr =  CLI_CUR_IND_KEY;
	key.dsize = strlen(key.dptr);

	data.dptr = (void *)&cur_ind;
	data.dsize = sizeof(cur_ind);

    return(gdbm_store(dbf, key, data, GDBM_REPLACE));
}

/*
 * Return the Current index of the history file
 * Returns a -1 if no entry found.
 */
long
FindCurInd(GDBM_FILE dbf)
{
	datum 		key, data;
	long		index;

	key.dptr =  CLI_CUR_IND_KEY;
	key.dsize = strlen(key.dptr);

	data = gdbm_fetch(dbf, key);

	if (data.dptr) {
		index = *(long *)data.dptr;
		Chkfree(data.dptr);
	}
	else {
		NETERROR(MRSD, ("FindCurInd: Could not find current index of Cli Hist DB\n"));
		index = -1;
	}

	return(index);
}

/*
 * Return the cli command entry for the provided timestamp from the history file
 * Returns a NULL if no entry found.
 * The calling function should free memory for the entry after its use.
 */
int
FindEntry(GDBM_FILE dbf, long seqno, char **entry, int *entrylenp)
{
	datum	key, data;

	key.dptr = (void *)&seqno;
	key.dsize = sizeof(seqno);

	data = gdbm_fetch(dbf, key);

	*entry = data.dptr;

	return (*entrylenp = data.dsize);
}

CliEntry **
FindCliEntries(HDB *hdbp, long seqno)
{
	int 		i;
	long 		curr_seqno;
	CliEntry	**clipp, **s;

	if ((s = (clipp = (CliEntry **)malloc(histdb_size*sizeof(CliEntry *)))) == NULL) {
		NETERROR(MRSD, ("FindCliEntries: malloc failed - %s\n", strerror(errno)));
		goto FCEnt_err;
	}
	curr_seqno = FindCurInd(hdbp->gdbmf);

	if ((curr_seqno - seqno)+1 > (histdb_size-1)) {
		NETDEBUG(MRSD, NETLOG_DEBUG1,
			("History DB of %d overrun: current seq = %ld, requested seq = %ld\n",
			histdb_size, curr_seqno, seqno));
		goto FCEnt_err;
	}

	for (i = seqno; i <= curr_seqno; i++) {
		*s = FindCliEntry(hdbp, i);
		if (*s == NULL) {
			NETERROR(MRSD, ("FindCliEntries: Break in history database found. Possible Corruption\n"));
			break;
		}
		s++;
	}

FCEnt_err:
	*s = NULL;

	return(clipp);
}

CliEntry *
FindCliEntry(HDB *hdbp, long seqno)
{
	CliEntry 	*clip = NULL;

	if ((clip = malloc(sizeof(CliEntry))) == NULL) {
		NETERROR(MRSD, ("FindCliEntry: malloc failed - %s\n", strerror(errno)));
		return(NULL);
	}
	if (FindEntry(hdbp->gdbmf, seqno, &(clip->clicmd), &(clip->cmdlen)) != 0)  {
		clip->seqno = seqno;
	}
	else {
		Chkfree(clip);
		clip = NULL;
	}

	return(clip);
}

void
PrintCliHist(HDB *hdbp)
{
	datum 	key, data;
	GDBM_FILE	dbf;
	long	cur_ind = -1;
	datum 	*tablep, *s;
	int		nkeys = 0, pages = 1, i, len;
	char 	*cmdstr = NULL, line[MAXLEN];
	Cmd 	*cmdp;

	dbf = hdbp->gdbmf;
	if ((s = tablep = malloc( (pages*CLI_MAX_HIST*(sizeof(datum))) )) == NULL) {
		NETERROR(MRSD, ("PrintCliHist: malloc failed - %s\n", strerror(errno)));
		return;
	}

	key = gdbm_firstkey(dbf);
	while (key.dptr) {

		if (nkeys == CLI_MAX_HIST) {
			/* More space is needed for the table */
			pages++;
			tablep = realloc(tablep, (pages*CLI_MAX_HIST*sizeof(datum)) );
			s = tablep + (pages-1)*CLI_MAX_HIST;
		}

		if ((key.dsize == strlen(CLI_CUR_IND_KEY)) && 
				(strncmp((char *)key.dptr, CLI_CUR_IND_KEY, strlen(CLI_CUR_IND_KEY)) == 0)) {
		/* Found the index key */
			data = gdbm_fetch(dbf, key);
			if (data.dsize == sizeof(long))
				cur_ind = *(long *)data.dptr;
			else
				NETERROR(MRSD, ("Index key found. Index not of right size. DB corrupt\n"));
		}
		else {
			*s++ = gdbm_fetch(dbf, key);
			nkeys++;
		}

		key = gdbm_nextkey(dbf, key);
	}

	qsort((void *)tablep, nkeys, sizeof(datum), CmdCompare);

	/* Print the Table */
	s  = tablep;
	printf("The Index key is %ld\n", cur_ind);

	printf(" SequenceNum Type Len   Pid        Time          Cmd String\n");
	for (i = 0; i < nkeys ; i++) {
		len = 0;
		data = *s++;
		cmdp = (Cmd *)data.dptr;
		(data.dsize != cmdp->cmdlen) ? (cmdstr = "NOT A VALID COMMAND") : 
			(cmdstr = (char *)(cmdp+1));
		len += snprintf(line+len, MAXLEN-len, "%10ld %4d ", cmdp->cmdseq, cmdp->cmdtyp); 
		len += snprintf(line+len, MAXLEN-len, "%5d %5d ", cmdp->cmdlen, cmdp->cmdpid); 
		// len += cftime(line+len, "%D %T ", (time_t *)&(cmdp->cmdtim)); replacing this with POSIX strftime.
		len += strftime(line+len, MAXLEN-len, "%D %T ", localtime((time_t *)&(cmdp->cmdtim)));
		len += snprintf(line+len, MAXLEN-len, "\t%s", cmdstr); 
		printf("%s\n", line);

		Chkfree(data.dptr);
	}
	Chkfree(tablep);
}

int
CmdCompare(const void *d1, const void *d2)
{
	datum dat1 = *((datum *)d1);
	datum dat2 = *((datum *)d2);

	Cmd 	*cmdp1, *cmdp2;

	cmdp1 = (Cmd *)dat1.dptr;
	cmdp2 = (Cmd *)dat2.dptr;

	if ( (cmdp1->cmdseq) > (cmdp2->cmdseq) )
		return(1);

	if ( (cmdp1->cmdseq) < (cmdp2->cmdseq) )
		return(-1);

	return(0);
}

void
FreeCliEntry(CliEntry *clip)
{
    Chkfree(clip->clicmd);
	Chkfree(clip);
}

void
FreeCliEntries(CliEntry **clipp)
{
	CliEntry	**s;

	s = clipp;
	while (*s)
		FreeCliEntry(*s++);
	Chkfree(clipp);
}

/*
 * GetSeqNum() returns a monotonically increasing timestamp
 * Returns -1 on failure.
 */
long
GetSeqNum(int fd, int operation)
{
	char    buf[80];
	int     n;
	long	seqno;

	/* Open the Cli History Database */

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		NETERROR(MRSD, ("GetSeqNum: read Failed - %s\n", strerror(errno)));
		return(-1);
 	}
	buf[n] = '\0';
	seqno = atol(buf);
	if (operation == SEQ_INC) {
		seqno++;
		snprintf(buf,sizeof(buf),"%ld",seqno);
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, strlen(buf));
	}

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("GetSeqNum:  op = %d, seqno = %ld\n", operation, seqno));
	return(seqno);
}

/*
 * LockSeqNum opens the sequence number file and locks it
 * The lock is exclusive and nonblocking. If file open
 * fails it returns (-1). If lock fails it returns (-2).
 * If lock succeeds it returns the file descriptor of the file
 */
int
LockSeqNum(void)
{

	int		fd;

#if 1
	HDBGetLock(HDB_seq);
#endif

	fd = open(CLI_SEQNUM_FILE, O_RDWR|O_CREAT, S_IRWXU);

	if (fd < 0) {
		NETERROR(MRSD, ("LockSeqNum: %s\n", strerror(errno)));
		return(RS_ERR_FOPEN);
	}

#if 0
	if ( flock(fd, LOCK_EX) != 0) {
		NETERROR(MRSD, ("flock failed on %s", CLI_SEQNUM_FILE));
		return(RS_ERR_FLOCK);
	}
#endif

	return(fd);

}

/*
 * Unlock and close the file
 */
void
UnlockSeqNum(int fd)
{
	close(fd); 	/* close release the flock also */
#if 1
	HDBRelLock(HDB_seq);
#endif
}

long
GetCliSeqNum()
{
	int 	fd;
	long	seqnum; 
		
	fd = LockSeqNum();
	seqnum = GetSeqNum(fd, SEQ_INC);
	UnlockSeqNum(fd);

	return(seqnum);
}

long
ReadSeqNum()
{
	int 	fd;
	long	seqnum; 
		
	fd = LockSeqNum();
	seqnum = GetSeqNum(fd, SEQ_RD);
	UnlockSeqNum(fd);

	return(seqnum);
}

void
gdbm_fatal(char *str)
{
	NETERROR(MRSD, ("gdbm error: %s\n", gdbm_strerror(gdbm_errno)));
}

/*
 * GetDBRevNum() returns a monotonically increasing database revision number
 * Returns -1 on failure.
 */
int
GetDBRevNum(int fd, int operation, void *arg)
{
	char    buf[80];
	int     n;
	int		dbrev;
	char	fn[] = "GetDBRevNum";

	/* Open the database revision number file */

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		NETERROR(MRSD, ("GetDBRevNum: read Failed - %s\n", strerror(errno)));
		return(-1);
 	}
	buf[n] = '\0';
	dbrev = atoi(buf);

	if (operation == DBREV_INC) {
		dbrev++;
	}
	else if (operation == DBREV_MOD) {
		if ( ( dbrev - (int) arg ) <= 0 ) {
			dbrev = (int) arg;
		}
		else {
			NETERROR(MRSD, ("%s: Attempt to lower revision number failed. cur - %d, att - %d\n",
				fn, dbrev, (int) arg));
		}
	}

	snprintf(buf, sizeof(buf), "%d", dbrev);
	lseek(fd, 0, SEEK_SET);
	write(fd, buf, strlen(buf));

	NETDEBUG(MRSD, NETLOG_DEBUG1, ("GetDBRevNum:  op = %d, dbrev = %d\n", 
		operation, dbrev));

	return(dbrev);
}

/*
 * LockDBRevNum opens the db revision number file and locks it
 * The lock is exclusive and nonblocking. If file open
 * fails it returns (-1). If lock fails it returns (-2).
 * If lock succeeds it returns the file descriptor of the file
 */
int
LockDBRevNum(void)
{

	int		fd;
	char	fn[] = "LockDBRevNum";

	HDBGetLock(HDB_dbrev);

	fd = open(CLI_DBREV_FILE, O_RDWR|O_CREAT, S_IRWXU);

	if (fd < 0) {
		NETERROR(MRSD, ("%s: %s\n", fn, strerror(errno)));
		return(RS_ERR_FOPEN);
	}

	return(fd);
}

/*
 * Unlock and close the file
 */
void
UnlockDBRevNum(int fd)
{
	close(fd); 	/* close release the flock also */

	HDBRelLock(HDB_dbrev);
}

int
GetNextDBRevNum(void)
{
	int 	fd;
	int		dbrev; 

	fd = LockDBRevNum();
	dbrev = GetDBRevNum(fd, DBREV_INC, NULL);
	UnlockDBRevNum(fd);

	return(dbrev);
}

int
ReadDBRevNum(void)
{
	int 	fd;
	int		dbrev; 
		
	fd = LockDBRevNum();
	dbrev = GetDBRevNum(fd, DBREV_RD, NULL);
	UnlockDBRevNum(fd);

	return(dbrev);
}

int
ModDBRevNum(int newrev)
{
	int 	fd;
	int		dbrev; 
		
	fd = LockDBRevNum();
	dbrev = GetDBRevNum(fd, DBREV_MOD, (void *)newrev);
	UnlockDBRevNum(fd);

	return(dbrev);
}

void
Chkfree(void *ptr)
{
	if (ptr) 
		free(ptr);
	else {
		NETDEBUG(MRSD, NETLOG_DEBUG2, ("Warning: Attempt to free a NULL pointer\n"));
	}
}
