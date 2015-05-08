#include "flock.h"
#include "unp.h"

#ifdef I86PC
#if 0
int
flock(int fd, int operation)
{
	int cmd;
	struct flock fl;

	cmd = F_SETLKW;

	switch (operation&(LOCK_SH|LOCK_EX|LOCK_UN))
	{
	case LOCK_SH:
		fl.l_type = F_RDLCK;
		break;
	case LOCK_EX:
		fl.l_type = F_WRLCK;
		break;
	case LOCK_UN:
		fl.l_type = F_UNLCK;
		break;
	default:
		break;
	}

	if (operation & LOCK_NB)
	{
		cmd = F_SETLK;
	}

	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	return fcntl(fd, cmd, &fl);
}
#endif
#endif

int 
HDBCreateLocks(void) 
{                                                                                        
	int semid;                                                                        
	char fn[] = "HDbCreateLock():";
                                                                                     
	if (smn_create(ftok(DBLOCKDIR, RSD_SEM_PROJ), HDB_max, 1, 0, &semid) < 0) {
		NETERROR(MRSD, ("%s could not create history database locks\n", fn));
		return(-1);                                                                      
	}                                                                                    
	return(0);                                                                           
}

int
HDBDeleteLocks(void)
{
	int semid;

	if (smn_get(ftok(DBLOCKDIR, RSD_SEM_PROJ), HDB_max, 1, 0, &semid) >= 0)
	{	
		smn_delete(semid);
	}

	return(0);
}

void
HDBGetLock(int dbid)
{
	int semid;	

	if (smn_get(ftok(DBLOCKDIR, RSD_SEM_PROJ), HDB_max, 1, 0, &semid) < 0) {
		NETERROR(MRSD, ("smn_get failed - %s \n", strerror(errno)));
	}
	else {
		smn_p(semid, dbid, 0, 0);
		NETDEBUG(MRSD, NETLOG_DEBUG4, ("pid %d:%d acquired locks for db %d\n", (int)getpid(), pthread_self(), dbid));
	}
}

void
HDBRelLock(int dbid)
{
	int semid;	

	if (smn_get(ftok(DBLOCKDIR, RSD_SEM_PROJ), HDB_max, 1, 0, &semid) < 0) {
		NETERROR(MRSD, ("sm_create failed\n"));
	}
	smn_v(semid, dbid);
	NETDEBUG(MDB, NETLOG_DEBUG4, ("pid %d:%d released locks for db %d on error\n", (int)getpid(), pthread_self(), dbid));
}
