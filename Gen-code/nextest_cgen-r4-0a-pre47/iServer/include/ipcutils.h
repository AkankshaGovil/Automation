#ifndef _ipc_utils_h_
#define _ipc_utils_h_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>

#ifdef SUNOS
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
} arg;
#else
#ifdef LIBC21
union semun {
	int val;                    /* value for SETVAL */
	struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;  /* array for GETALL, SETALL */
	struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif
#endif

int
q_vcreate(
        key_t key,
        int flags,
        unsigned long maxnum,
        unsigned long maxlen,
        int *msqid
);

int
q_vget(
        key_t key,
        int flags,
        unsigned long maxnum,
        unsigned long maxlen,
        int *msqid
);

int
q_vsend(
        int msqid,
        void *msgbuf,
        size_t nbytes,
        int flags
);

ssize_t
q_vreceive(
        int msqid,
        void *msgbuf,
        size_t nbytes,  /* size of buffer */
        long type,
        int flag,
        ssize_t *msglen     /* size of actual message received */
);

int
q_vdelete(
        int msqid
);

int
sm_create(
        key_t key,
        unsigned long count,
        unsigned long flags,
        int *semid
);

int
sm_get(
        key_t key,
        unsigned long count,
        unsigned long flags,
        int *semid
);

/* Perform the 'P' operation on a semaphore */
/* flags = IPC_NOWAIT, then no block.
 * timeout not used.
 * return value of 0 = no error,
 * of -1, means non-block errors in cases of IPC_NOWAIT,
 * and real errors in other cases.
 */
int
sm_p(
        int semid,
        short flags,
        unsigned long timeout   /* NOT USED */
);

int
sm_v(
        int semid
);

int
sm_delete(
        int semid
);

int
smn_create(
        key_t key,
		int nsems,
        unsigned long count,
        unsigned long flags,
        int *semid
);

int
smn_get(
        key_t key,
		int	nsems,
        unsigned long count,
        unsigned long flags,
        int *semid
);

int
smn_p(
        int semid,
		int semnum,
        short flags,
        unsigned long timeout   /* NOT USED */
);

int
smn_v(
        int semid,
		int semnum
);

int
smn_delete(
        int semid
);

extern int q_vopen (key_t key, int flags, int *msqid);
extern int q_vdelete (int msqid);

#endif /* _ipc_utils_h_ */
