#ifndef __flock_h
#define __flock_h

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include "ipcutils.h"
#include "ipckey.h"
#include "srvrlog.h"

#define LOCK_SH 1    /* Shared lock.  */                                    
#define LOCK_EX 2    /* Exclusive lock.  */                                 
#define LOCK_UN 8    /* Unlock.  */                                         
#define LOCK_NB 4    /* Don't block when locking.  */    

typedef enum {
	HDB_seq = 0,
	HDB_hist,
	HDB_dbrev,

	HDB_max
} HDB_tTypes;
	
#ifdef I86PC
extern int flock(int fd, int operation);
#endif

int 	HDBCreateLock(void);
int 	HDBDeleteLock(void);
void 	HDBGetLock(int dbid);
void 	HDBRelLock(int dbid);
#endif /* 	__flock_h */
