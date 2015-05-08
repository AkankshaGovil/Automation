#include <stdio.h>
#include <db.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>

#define USE_TXN 1

#define NUM_DB 2
#define REC_SIZE 1024

char rad_dirname[500];

#define DATABASE "radacct.db"
#define NUM_READER_THREADS 10
#define NUM_WRITER_THREADS 10
#define INITIAL_RECORDS 40000
#define NUM_SECONDS 100
#define REC_SIZE 1024
#define TRUE 1
#define FALSE 0

static pthread_mutex_t radacctMutex = PTHREAD_MUTEX_INITIALIZER;

static DB_ENV *radacct_dbenv;	/* Database environment. */
static DB *radacct_dbp;			/* Database handles. */
void removeInfo(DB_TXN *tid);
void* reader_thread(void*);
void* writer_thread(void*);
void terminate_threads(int);
void storeInfo(char* buf, int id);
int getInfo(int id);
void initialize_buf(struct tms *buf);

static int radacctinPool;
static int radacctinClass;
int inserted=0;
int removed=0;

pthread_t reader_threads[NUM_READER_THREADS];
pthread_t writer_threads[NUM_WRITER_THREADS];
static pthread_mutex_t mu= PTHREAD_MUTEX_INITIALIZER;
clock_t put_total_time[NUM_WRITER_THREADS+1];
clock_t get_total_time[NUM_READER_THREADS+1];
clock_t remove_total_time[NUM_READER_THREADS+1];
char data[REC_SIZE];
struct tms put_time_buf[NUM_WRITER_THREADS+1];
struct tms get_time_buf[NUM_READER_THREADS+1];
struct tms remove_time_buf[NUM_READER_THREADS+1];
long reader_numData[NUM_READER_THREADS];
long writer_numData[NUM_WRITER_THREADS];
main()
{
	int number,count;
	int reader_taskids[NUM_READER_THREADS];
	int writer_taskids[NUM_WRITER_THREADS];
	pthread_attr_t attr;
	
	initRadacct_db(&number); //Initialize the database
 	pthread_attr_init(&attr);	
        pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
        pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
	
	
	for(count=0; count < REC_SIZE; ++count)
	{
		data[count] = (char)107;
	}
	data[REC_SIZE-1] = '\0';

	//Initially store some records
	for(count=0; count<INITIAL_RECORDS; ++count)
	{
		storeInfo(data,NUM_WRITER_THREADS);  //Pass NUM_THREADS as time of this index is not calculated
	}

	for(count=0;count < NUM_READER_THREADS;++count){
		reader_taskids[count]=count;
		get_total_time[count]=0;
		remove_total_time[count]=0;
		initialize_buf(&get_time_buf[count]);
		initialize_buf(&remove_time_buf[count]);
		reader_numData[count]=0;
		writer_taskids[count]=count;
		put_total_time[count]=0;
		initialize_buf(&put_time_buf[count]);
		writer_numData[count]=0;
		pthread_create(&writer_threads[count], &attr, writer_thread,(void*)writer_taskids[count]);
		pthread_create(&reader_threads[count], &attr, reader_thread,(void*)reader_taskids[count]);
			
	}
	
	alarm(NUM_SECONDS);
	signal(SIGALRM, terminate_threads);
	while(TRUE); //Looping till main receives SIGALRM
}

void initialize_buf(struct tms *buf)
{
	buf->tms_utime = 0;
	buf->tms_stime = 0;
	buf->tms_cutime = 0;
	buf->tms_cstime = 0;
}

void terminate_threads(int sig_num)
{
  int count, inserted=0, removed=0; 
  int readerTotalTime=0, writerTotalTime=0, removeTotalTime=0;
  int readerTotalutime=0, readerTotalstime=0;
  int writerTotalutime=0, writerTotalstime=0;
  int removeTotalutime=0, removeTotalstime=0;
  int clock_tic_per_sec=0;
  printf("\n\n-------------------- 100 seconds over------------\n\n");
  for(count=0;count < NUM_READER_THREADS; ++count)
  {
	pthread_cancel(reader_threads[count]);
	pthread_cancel(writer_threads[count]);
  }
  printf("\n\nDone cancelling all threads\n\n");
  for(count=0; count < NUM_READER_THREADS; ++count)
  {
		  removed+=reader_numData[count];
		  readerTotalTime+=get_total_time[count];
		  readerTotalutime+=get_time_buf[count].tms_utime;
		  readerTotalstime+=get_time_buf[count].tms_stime;
		  
		  removeTotalTime+=remove_total_time[count];
		  removeTotalutime+=remove_time_buf[count].tms_utime;
		  removeTotalstime+=remove_time_buf[count].tms_stime;
		  
		  inserted+=writer_numData[count];
		  writerTotalTime+=put_total_time[count];
		  writerTotalutime+=put_time_buf[count].tms_utime;
		  writerTotalstime+=put_time_buf[count].tms_stime;
  }
  clock_tic_per_sec = sysconf(_SC_CLK_TCK);
  printf("Total number of records removed: %d\n", removed);
  printf("Total number of records inserted: %d\n\n", inserted);
  printf("Average time for put: %f\n", ((float)(writerTotalTime)/(inserted*clock_tic_per_sec)));
  printf("Average user time for put: %f\n",((float)(writerTotalutime)/(inserted*clock_tic_per_sec)));
  printf("Average system time for put: %f\n\n",((float)(writerTotalstime)/(inserted*clock_tic_per_sec)));
  printf("Average time for get: %f\n", ((float)(readerTotalTime)/(removed*clock_tic_per_sec)));
  printf("Average user time for get: %f\n",((float)(readerTotalutime)/(removed*clock_tic_per_sec)));
  printf("Average system time for get: %f\n\n",((float)(readerTotalstime)/(removed*clock_tic_per_sec)));
  
  printf("Average time for commit: %f\n", ((float)(removeTotalTime)/(removed*clock_tic_per_sec)));
  printf("Average user time for commit: %f\n",((float)(removeTotalutime)/(removed*clock_tic_per_sec)));
  printf("Average system time for commit: %f\n",((float)(removeTotalstime)/(removed*clock_tic_per_sec)));
  
  pthread_exit((void*)0);  
}

void* writer_thread(void* threadNum)
{
	while(TRUE){
	storeInfo(data, (int)threadNum);
	writer_numData[(int)threadNum]++;
	}
}
		

void* reader_thread(void* threadNum)
{
        while(TRUE){	
	getInfo((int)threadNum);
	reader_numData[(int)threadNum]++;
	}
}



static int db_init_env(DB_ENV **dbenv, const char *home)
{
	int ret;
	char abs_home[256];

	snprintf(abs_home, sizeof(abs_home), "%s/%s", getenv("PWD"), home);

	if((ret = mkdir(abs_home, 0755)) == -1)
	{
		if(errno != EEXIST)
		{
		 	printf("db_env_create: can not open directory path: %s\n", abs_home);
			return 1;
		}
	}

	if((ret = db_env_create(dbenv, 0)) != 0)
	{
		printf("db_env_create: %s\n", db_strerror(ret));
		return 1;
	}

	(*dbenv)->set_errfile(*dbenv, stderr);
	(*dbenv)->set_errpfx(*dbenv, "gis");
	(void)(*dbenv)->set_cachesize(*dbenv, 0, 100 * 1024, 0);
	(void)(*dbenv)->set_lg_max(*dbenv, 200000);
	(void)(*dbenv)->set_tx_max(*dbenv, 300);
	(void)(*dbenv)->set_flags(*dbenv, DB_DIRECT_DB, 1);
	//(void)(*dbenv)->set_flags(*dbenv, DB_DIRECT_LOG, 1);
	//(void)(*dbenv)->set_flags(*dbenv, DB_TXN_NOSYNC, 1);

	if((ret = (*dbenv)->open(*dbenv, abs_home,
			DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
				DB_INIT_MPOOL | DB_INIT_TXN | DB_THREAD | DB_RECOVER, 0)) != 0)
	{
		(*dbenv)->err(*dbenv, ret, NULL);
		(void)(*dbenv)->close(*dbenv, 0);
		return 1;
	}

	return 0;
}


static int db_init(DB_ENV *dbenv, DB **dbp)
{
	int ret;

	/* Initialize the database. */
	if((ret = db_create(dbp, dbenv, 0)) != 0)
	{
		dbenv->err(dbenv, ret, "db_create");
		(void)dbenv->close(dbenv, 0);
		return 1;
	}

	if((ret = (*dbp)->set_re_len(*dbp, REC_SIZE)) != 0)
	{
		(*dbp)->err(*dbp, ret, "set_re_source");
		goto err;
	}

	if((ret = (*dbp)->set_q_extentsize(*dbp, 1000)) != 0)
	{
		(*dbp)->err(*dbp, ret, "set_q_extentsize");
		goto err;
	}

	if((ret = (*dbp)->open(*dbp, NULL, DATABASE, NULL, DB_QUEUE,
					DB_AUTO_COMMIT | DB_CREATE | DB_THREAD, 0664)) != 0)
	{
		(*dbp)->err(*dbp, ret, "%s: open", DATABASE);
		goto err;
	}

	return 0;

err:
	(void)(*dbp)->close(*dbp, 0);
	(void)dbenv->close(dbenv, 0);

	return 1;
}



int initRadacct_db(int *num_current)
{

	int i, ret;
	pthread_t tid;
	DB *dbp;
	DB_QUEUE_STAT *stat;

	if((ret = db_init_env(&radacct_dbenv,"database")) == 0)
	{
		if((ret = db_init(radacct_dbenv, &radacct_dbp)) != 0)
		{
			return -1;
		}

		dbp = radacct_dbp;

		if((ret = dbp->stat(dbp, &stat, 0)) == 0)
		{
			*num_current = stat->qs_ndata;

			free(stat);
		}
	}
	else
	{
		return -1;
	}

	return 0;
}


void closeRadacct_db()
{
	int i, ret;

		if((ret = radacct_dbp->close(radacct_dbp, 0)) != 0)
		{
			printf("radacct dp close failed: %d\n", i, ret);
		}

		if((ret = radacct_dbenv->close(radacct_dbenv, 0)) != 0)
		{
			printf("radacct env close failed: %d\n", i, ret);
		}
}
	

void storeInfo(char* buf, int id)
{
	DB *dbp = radacct_dbp;
	DBT key, data;
	int ret;
	clock_t t1, t2;
	struct tms buf1, buf2;


	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.flags = DB_DBT_MALLOC;

	data.data = buf;
	data.size = data.ulen = sizeof(buf);
	data.flags = DB_DBT_USERMEM;
	
	t1 = times(&buf1);
	ret = dbp->put(dbp, NULL, &key, &data, DB_AUTO_COMMIT | DB_APPEND);
	t2 = times(&buf2);
	put_total_time[id] += (t2 - t1);
	put_time_buf[id].tms_utime += (buf2.tms_utime - buf1.tms_utime);
	put_time_buf[id].tms_stime += (buf2.tms_stime - buf1.tms_stime);
	put_time_buf[id].tms_cutime += (buf2.tms_cutime - buf1.tms_cutime);
	put_time_buf[id].tms_cstime += (buf2.tms_cstime - buf1.tms_cstime);
	switch(ret)
	{
		case DB_LOCK_DEADLOCK:
			printf("storeInfo: deadlock: %s\n", db_strerror(ret));
			break;

		case 0:
			break;

		default:
			printf("storeInfo: oops: %d\n", ret);
			break;
	}	

	if(key.data) free(key.data);

}


int getInfo(int id)
{
	DB_ENV *dbenv = radacct_dbenv;
	DB *dbp = radacct_dbp;
	DB_TXN *tid = NULL;
	DBT key, data;
	db_recno_t recno;
	int info = 0;
	char buf[REC_SIZE];
	int ret;
	clock_t t1, t2;
	clock_t t3, t4;
	struct tms buf1, buf2;
	struct tms buf3, buf4;

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = &recno;
	key.size = key.ulen = sizeof(recno);
	key.flags = DB_DBT_USERMEM;

	data.data = buf;
	data.ulen = sizeof(buf);
	data.flags = DB_DBT_USERMEM;

	if((ret = dbenv->txn_begin(dbenv, NULL, &tid, 0)) != 0)
	{
		printf("getInfo: transaction failed: %s\n", db_strerror(ret));
		return 0;
	}
	t1 = times(&buf1);
	ret = dbp->get(dbp, tid, &key, &data, DB_CONSUME);
	t2 = times(&buf2);
	get_total_time[id] += (t2 - t1);
	get_time_buf[id].tms_utime += (buf2.tms_utime - buf1.tms_utime);
	get_time_buf[id].tms_stime += (buf2.tms_stime - buf1.tms_stime);
	get_time_buf[id].tms_cutime += (buf2.tms_cutime - buf1.tms_cutime);
	get_time_buf[id].tms_cstime += (buf2.tms_cstime - buf1.tms_cstime);
	switch(ret)
	{

		case DB_LOCK_DEADLOCK:
			printf("getInfo: deadlock: %s\n", db_strerror(ret));
			break;

		case 0:
			t3 = times(&buf3);	
			removeInfo(tid);
			t4 = times(&buf4);
			remove_total_time[id] = (t4 - t3);
			remove_time_buf[id].tms_utime += (buf4.tms_utime - buf3.tms_utime);
			remove_time_buf[id].tms_stime += (buf4.tms_stime - buf3.tms_stime);
			remove_time_buf[id].tms_cutime += (buf4.tms_cutime - buf3.tms_cutime);
			remove_time_buf[id].tms_cstime += (buf4.tms_cstime - buf3.tms_cstime);
			break;

		default:
			printf("getInfo: oops: %d\n", ret);
			break;
	}

	return info;
}


void removeInfo(DB_TXN *tid)
{
	int ret;

	if(tid)
	{
		if((ret = tid->commit(tid, 0)) != 0)
		{
			printf("removeInfo: commit failed: %s\n", db_strerror(ret));
		}
	}
}
