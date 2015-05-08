#include <stdio.h>
#include "queue.h"
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


void removeInfo(DbTxn tid);
void* reader_thread(void*);
void* writer_thread(void*);
void terminate_threads(int);
void storeInfo(char* buf, int id);
int getInfo(int id);
void initialize_buf(struct tms *buf);
int initRadacct_db(int *num_current);

static int radacctinPool;
static int radacctinClass;

pthread_t reader_threads[NUM_READER_THREADS];
pthread_t writer_threads[NUM_WRITER_THREADS];
clock_t put_total_time[NUM_WRITER_THREADS+1];
clock_t get_total_time[NUM_READER_THREADS+1];
clock_t remove_total_time[NUM_READER_THREADS+1];
char data[REC_SIZE];
struct tms put_time_buf[NUM_WRITER_THREADS+1];
struct tms get_time_buf[NUM_READER_THREADS+1];
struct tms remove_time_buf[NUM_READER_THREADS+1];
long reader_numData[NUM_READER_THREADS];
long writer_numData[NUM_WRITER_THREADS];

DbEnv env;

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
	printf("initialization done..\n");

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

	return(0);
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
  long count, inserted=0, removed=0; 
  clock_t readerTotalTime=0, writerTotalTime=0, removeTotalTime=0;
  clock_t readerTotalutime=0, readerTotalstime=0;
  clock_t writerTotalutime=0, writerTotalstime=0;
  clock_t removeTotalutime=0, removeTotalstime=0;
  long clock_tic_per_sec=0;
  printf("\n\n-------------------- 100 seconds over------------\n\n");
  for(count=0;count < NUM_READER_THREADS; ++count)
  {
	pthread_cancel(reader_threads[count]);
	pthread_cancel(writer_threads[count]);
  }

  for(count=0;count < NUM_READER_THREADS; ++count)
  {
	pthread_join(reader_threads[count],NULL);
	pthread_join(writer_threads[count],NULL);
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
  printf("Total number of records removed: %ld\n", removed);
  printf("Total number of records inserted: %ld\n\n", inserted);
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
  printf("Called pthread_exit\n");
}

void* writer_thread(void* threadNum)
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  while(TRUE){
    //		printf("writer thread: %d\n", (int)threadNum);
    storeInfo(data, (int)threadNum);
    //	writer_numData[(int)threadNum]++;
  }
}
		

void* reader_thread(void* threadNum)
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  while(TRUE){	
    //printf("reader thread: %d\n", (int)threadNum);
    getInfo((int)threadNum);
    //	reader_numData[(int)threadNum]++;
  }
}



static int db_init()
{
	int ret;
	char abs_home[256];
	int error;

	env = (DbEnv) malloc(sizeof(DbEnvStruct));
        env->databaseName = (char*)malloc(100);
        env->pathName = (char*)malloc(100);
        strcpy(env->databaseName, "database");
        strcpy(env->pathName, getcwd(abs_home, 256));
        env->numRecords = 2000;
        env->queueSize = 700;
	env->initialFileSize = 1024*3000;

        //Initialize database
        ret = DbInit(env, &error);

	return(ret);
}


int initRadacct_db(int *num_current)
{

	int i, ret;
	pthread_t tid;
	

	if((ret = db_init()) != 0)
	{
			return -1;
	}


	return 0;
}


void closeRadacct_db()
{
	int i, ret;
	int error;

		if((ret = DbClose(env, &error)) != 0)
		{
			printf("db close failed: %d\n", ret);
		}

}
	

void storeInfo(char* buf, int id)
{
	int ret;
	clock_t t1, t2;
	struct tms buf1, buf2;
	Data data;
	int error;

	data = (Data) malloc(sizeof(DataStruct));
	data->data = (char*) malloc(REC_SIZE + 20);
	sprintf(data->data, "%s.%d", buf, id);
	data->len = strlen(data->data);

	
	t1 = times(&buf1);
	ret = DbPut(env, data, &error);
	t2 = times(&buf2);
	put_total_time[id] += (t2 - t1);
	put_time_buf[id].tms_utime += (buf2.tms_utime - buf1.tms_utime);
	put_time_buf[id].tms_stime += (buf2.tms_stime - buf1.tms_stime);
	put_time_buf[id].tms_cutime += (buf2.tms_cutime - buf1.tms_cutime);
	put_time_buf[id].tms_cstime += (buf2.tms_cstime - buf1.tms_cstime);
	switch(ret)
	{
		case -1:
		  /*printf("storeInfo: error");*/
			break;

		case 0:
		        writer_numData[id]++;
			break;

	}	


}


int getInfo(int id)
{
	DbTxn txn;
	Data data;
	int error;
	int info = 0;
	char buf[REC_SIZE];
	int ret;
	clock_t t1, t2;
	clock_t t3, t4;
	struct tms buf1, buf2;
	struct tms buf3, buf4;


	t1 = times(&buf1);
	ret = DbGet(env, &data, &txn, &error);
	t2 = times(&buf2);
	get_total_time[id] += (t2 - t1);
	get_time_buf[id].tms_utime += (buf2.tms_utime - buf1.tms_utime);
	get_time_buf[id].tms_stime += (buf2.tms_stime - buf1.tms_stime);
	get_time_buf[id].tms_cutime += (buf2.tms_cutime - buf1.tms_cutime);
	get_time_buf[id].tms_cstime += (buf2.tms_cstime - buf1.tms_cstime);


	switch(ret)
	{

		case -1:
			printf("getInfo: failed");
			break;

		case 0:
			t3 = times(&buf3);	
			removeInfo(txn);
			t4 = times(&buf4);
			remove_total_time[id] += (t4 - t3);
			remove_time_buf[id].tms_utime += (buf4.tms_utime - buf3.tms_utime);
			remove_time_buf[id].tms_stime += (buf4.tms_stime - buf3.tms_stime);
			remove_time_buf[id].tms_cutime += (buf4.tms_cutime - buf3.tms_cutime);
			remove_time_buf[id].tms_cstime += (buf4.tms_cstime - buf3.tms_cstime);
			reader_numData[id]++;
			free(data->data);
			free(data);
			break;

		default:
			printf("getInfo: oops: %d\n", ret);
			break;
	}

	return(0);
}


void removeInfo(DbTxn tid)
{
	int ret;
	int error;

	if((ret = DbCommit(tid, &error)) != 0)
	{
		printf("removeInfo: commit failed\n");
		printf("File Number %d Offset :%"PRId32"\n", ((FileInfo) tid->fileHandle)->index, tid->offset);
	}
	free(tid);
}
