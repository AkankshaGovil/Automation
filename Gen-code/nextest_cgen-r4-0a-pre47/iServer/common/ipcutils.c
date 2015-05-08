#include <unistd.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <syslog.h>
#include <stdio.h>

#ifdef SUNOS
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
} arg;
#else
/* #ifdef LIBC21	was needed only for linux 5.2 */
union semun {
	int val;                    /* value for SETVAL */
	struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;  /* array for GETALL, SETALL */
	struct seminfo *__buf;      /* buffer for IPC_INFO */
};
/* #endif */
#endif

int
q_vcreate(
        key_t key,
        int flags,
        unsigned long maxnum,
        unsigned long maxlen,
        int *msqid
)
{
        int id;
        struct msqid_ds buf;

        *msqid = -1;

        id = msgget(key, flags|O_RDWR|IPC_CREAT);

        if (id < 0)
        {
                return -1;
        }

        *msqid = id;

        if (msgctl(id, IPC_STAT, &buf) < 0)
        {
                return -1;
        }

        buf.msg_qbytes = maxnum*maxlen;

        if (msgctl(id, IPC_SET, &buf) < 0)
        {
                return -1;
        }

        return 0;
}

int
q_vget(
        key_t key,
        int flags,
        unsigned long maxnum,
        unsigned long maxlen,
        int *msqid
)
{
        int id;
        struct msqid_ds buf;

        *msqid = -1;

        id = msgget(key, flags|O_RDWR|IPC_CREAT);

        if (id < 0)
        {
			return q_vcreate(key, flags, maxnum, maxlen, msqid);
        }

        *msqid = id;

        if (msgctl(id, IPC_STAT, &buf) < 0)
        {
                return -1;
        }

        buf.msg_qbytes = maxnum*maxlen;

        if (msgctl(id, IPC_SET, &buf) < 0)
        {
                return -1;
        }

        return 0;
}

int
q_vopen(
        key_t key,
        int flags,
        int *msqid
)
{
        int id;
        struct msqid_ds buf;

        *msqid = -1;

        id = msgget(key, flags|O_RDWR);

        if (id < 0)
        {
			return -1;		
        }

        *msqid = id;

        return 0;
}

int
q_vsend(
        int msqid,
        void *msgbuf,
        size_t nbytes,
        int flags
)
{
        int rc;

        rc = msgsnd(msqid, msgbuf, nbytes-4, flags);

        return rc;
}

ssize_t
q_vreceive(
        int msqid,
        void *msgbuf,
        size_t nbytes,  /* size of buffer */
        long type,
        int flag,
        ssize_t *msglen     /* size of actual message received */
)
{
        ssize_t mlen;

        if (!msglen)
        {
                msglen = &mlen;
        }

        *msglen = msgrcv(msqid, msgbuf, nbytes-4, type, flag);

        return *msglen;
}

int
q_vdelete(
        int msqid
)
{
        struct msqid_ds buf;

        return msgctl(msqid, IPC_RMID, &buf);
}

int
sm_create(
        key_t key,
        unsigned long count,
        unsigned long flags,
        int *semid
        )
{
        int id;
        union semun arg = {0};

        *semid = -1;

        id = semget(key, 1, O_RDWR|IPC_CREAT);
        if (id <0)
        {
                return -1;
        }

        *semid = id;

        arg.val = count;
        if (semctl(id, 0, SETVAL, arg) < 0)
        {
                return -1;
        }

        return 0;
}

int
sm_get(
        key_t key,
        unsigned long count,
        unsigned long flags,
        int *semid
        )
{
        int id;
        union semun arg = {0};

        *semid = -1;

        id = semget(key, 1, O_RDWR);
        if (id <0)
        {
        	return sm_create(key, count, flags, semid);
        }

        *semid = id;

        return 0;
}

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
)
{
        struct sembuf semoparray[1] = {{0,-1,0}};
        int rc;

        semoparray[0].sem_flg = flags;

        rc = semop(semid, semoparray, 1);

        return rc;
}

int
sm_v(
        int semid
)
{
        struct sembuf semoparray[1] = {{0,1,0}};
        int rc;

        //semoparray[0].sem_flg |= SEM_UNDO;
        rc = semop(semid, semoparray, 1);

        return rc;
}

int
sm_delete(
        int semid
)
{
        union semun arg = {0};

        return semctl(semid, 0, IPC_RMID, arg);
}

/* operations for semaphore arrays */
int
smn_create(
        key_t key,
		int nsems,
        unsigned long count,
        unsigned long flags,
        int *semid
        )
{
        int id, i;
        union semun arg = {0};

        *semid = -1;

        id = semget(key, nsems, O_RDWR|IPC_CREAT);
        if (id <0)
        {
                return -1;
        }

        *semid = id;

        arg.val = count;
		for (i=0; i<nsems; i++)
		{
        	if (semctl(id, i, SETVAL, arg) < 0)
        	{
                return -1;
        	}
		}

        return 0;
}

int
smn_get(
        key_t key,
		int	nsems,
        unsigned long count,
        unsigned long flags,
        int *semid
        )
{
        int id;
        union semun arg = {0};

        *semid = -1;

        id = semget(key, nsems, O_RDWR);
        if (id <0)
        {
        	return smn_create(key, nsems, count, flags, semid);
        }

        *semid = id;

        return 0;
}

/* Perform the 'P' operation on a semaphore */
/* flags = IPC_NOWAIT, then no block.
 * timeout not used.
 * return value of 0 = no error,
 * of -1, means non-block errors in cases of IPC_NOWAIT,
 * and real errors in other cases.
 */
int
smn_p(
        int semid,
		int semnum,
        short flags,
        unsigned long timeout   /* NOT USED */
)
{
        struct sembuf semoparray[1] = {{0,-1,0}};
        int rc;

		semoparray[0].sem_num = semnum;
        semoparray[0].sem_flg = flags;

        rc = semop(semid, semoparray, 1);

        return rc;
}

int
smn_v(
        int semid,
		int semnum
)
{
        struct sembuf semoparray[1] = {{0,1,0}};
        int rc;

		semoparray[0].sem_num = semnum;
        //semoparray[0].sem_flg |= SEM_UNDO;
        rc = semop(semid, semoparray, 1);

        return rc;
}

int
smn_delete(
        int semid
)
{
        union semun arg = {0};

        return semctl(semid, 0, IPC_RMID, arg);
}
