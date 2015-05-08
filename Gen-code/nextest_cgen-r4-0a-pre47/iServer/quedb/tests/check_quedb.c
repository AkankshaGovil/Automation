#include <check.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include "queue.h"
#define MAX_RECORDS_FILE 1000
#define NUM_PUT_THREADS  100
#define NUM_READER_THREADS 100
#define NUM_WRITER_THREADS 100
#define NUM_MT_RECORDS 500
#define NUM_RECORDS 500
#define TESTDBSDIR "/testdbs"


Suite *s;
TCase *tc1, *tc2,*tc3,*tc4;
SRunner *sr;
DbEnv env;
Db database;
pthread_mutex_t writeDataLock= PTHREAD_MUTEX_INITIALIZER;
int readDt[NUM_MT_RECORDS];
uint32_t writeDataNum=0;

START_TEST(test_NewDb){
	DbEnv newDbEnv;
	char buf[MAX_PATH_LEN];
	int ret;
	int error;
	Db database;
	newDbEnv = (DbEnv) malloc(sizeof(DbEnvStruct));
	newDbEnv->databaseName = (char*)malloc(100);
	newDbEnv->pathName = (char*)malloc(100);
	strcpy(newDbEnv->databaseName, "testNewDb");
	getcwd(buf, MAX_PATH_LEN);
        strcpy(&buf[strlen(buf)],TESTDBSDIR); 
	strcpy(newDbEnv->pathName, buf);
	newDbEnv->numRecords = MAX_RECORDS_FILE;
	newDbEnv->queueSize = 7;
	newDbEnv->initialFileSize = 1024*1024;
	
	ret = DbInit(newDbEnv, &error);

	fail_unless(ret == 0, "DbInit failed");

	fail_unless(newDbEnv->dbHandle != 0, "Db handle is null");

	database = (Db)newDbEnv->dbHandle;

	fail_unless(database != NULL, "Database is null");
	fail_unless(listItems(database->fileInfoList) == 1, "Spurious files in file info list");
	fail_unless(listItems(database->dataList) == 0, "Invalid data in data info list");
	fail_unless(database->fileInfoCurrent == ListGetNext(database->fileInfoList), "Invalid File info current");
	fail_unless(database->readOffset == 0, "Invalid read offset");
	fail_unless(database->writeOffset == 0, "Invalid write offset");
	fail_unless(database->env == newDbEnv, "Invalid env within db structure");
	
	ret = DbClose(newDbEnv, &error);
	fail_unless(ret == 0, "DbClose failed");

	database = (Db)newDbEnv->dbHandle;

	fail_unless(database == NULL, "Invalid database handle after closing database");
	
}
END_TEST	


START_TEST(test_putGet){
	DbEnv dbEnv;
	char buf[MAX_PATH_LEN];
	int ret;
	int error;
	Db database;
	Data data;
	DbTxn txn;
	char *str = "nextone";
	
	dbEnv = (DbEnv) malloc(sizeof(DbEnvStruct));
	dbEnv->databaseName = (char*)malloc(100);
	dbEnv->pathName = (char*)malloc(100);
	strcpy(dbEnv->databaseName, "DbGetPut");
	getcwd(buf, MAX_PATH_LEN);
        strcpy(&buf[strlen(buf)],TESTDBSDIR); 
	strcpy(dbEnv->pathName, buf);
	dbEnv->numRecords = MAX_RECORDS_FILE;
	dbEnv->queueSize = 7;
	dbEnv->initialFileSize = 1024*1024;
	
	ret = DbInit(dbEnv, &error);

	fail_unless(ret == 0, "DbInit failed");
	
	data = (Data) malloc(sizeof(DataStruct));
	data->data = (char*) malloc(80);
	strcpy(data->data, str);
	data->len = strlen(data->data);

	ret = DbPut(dbEnv, data, &error);
	fail_unless(ret == 0, "DbPut failed");

	free(data->data);
	free(data);

	ret = DbGet(dbEnv, &data, &txn, &error);
	fail_unless(ret == 0, "DbGet failed");

	database = (Db)dbEnv->dbHandle;
	fail_unless(database != NULL, "Database is null");

	fail_unless(database->readOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid read offset");
	fail_unless(database->writeOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid write offset");
	fail_unless(database->fileInfoCurrent == ListGetNext(database->fileInfoList), "Invalid file info current");
	fail_unless(listItems(database->fileInfoList) == 1, "Invalid file info list");
	fail_unless(listItems(database->dataList) == 0, "Invalid Data list");
	fail_unless(memcmp(str, data->data, data->len) == 0, "Invalid data from get");
	fail_unless(txn->offset == 0, "Invalid transaction offset");
	fail_unless(((FileInfo)(txn->fileHandle))->index == 0, "Invalid index from transaction file info");
	fail_unless(txn->dbHandle == dbEnv->dbHandle, "Invalid db handle in transaction");
	fail_unless(((FileInfo)(txn->fileHandle))->numRecords == 1, "Invalid number of records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->numActiveRecords == 1, "Invalid number of active records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->fd != -1, "Invalid file poiinter in transaction file info");

	free(data->data);
	free(data);
	free(txn);

	ret = DbClose(dbEnv, &error);
	fail_unless(ret == 0, "DbClose failed");

	database = (Db)dbEnv->dbHandle;

	fail_unless(database == NULL, "Invalid database handle after closing database");

}
END_TEST


START_TEST(test_singleRecordFile){
	DbEnv dbEnv;
	char buf[MAX_PATH_LEN];
	int ret;
	int error;
	Db database;
	Data data;
	DbTxn txn;
	char *str = "nextone";
	
	dbEnv = (DbEnv) malloc(sizeof(DbEnvStruct));
	dbEnv->databaseName = (char*)malloc(100);
	dbEnv->pathName = (char*)malloc(100);
	strcpy(dbEnv->databaseName, "DbSingleRecord");
	getcwd(buf, MAX_PATH_LEN);
        strcpy(&buf[strlen(buf)],TESTDBSDIR); 
	strcpy(dbEnv->pathName, buf);
	dbEnv->numRecords = 1;
	dbEnv->queueSize = 1;
	dbEnv->initialFileSize = 1024*1024;
	
	ret = DbInit(dbEnv, &error);

	fail_unless(ret == 0, "DbInit failed");
	
	data = (Data) malloc(sizeof(DataStruct));
	data->data = (char*) malloc(80);
	strcpy(data->data, str);
	data->len = strlen(data->data);

	ret = DbPut(dbEnv, data, &error);
	fail_unless(ret == 0, "DbPut failed");

	ret = DbPut(dbEnv, data, &error);
	fail_unless(ret == 0, "DbPut failed while inserting second time");

	free(data->data);
	free(data);

	ret = DbGet(dbEnv, &data, &txn, &error);
	fail_unless(ret == 0, "DbGet failed");

	database = (Db)dbEnv->dbHandle;
	fail_unless(database != NULL, "Database is null");

	fail_unless(database->readOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid read offset");
	fail_unless(database->writeOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid write offset");
	fail_unless(database->fileInfoCurrent == ListGetNext(database->fileInfoList), "Invalid file info current");
	fail_unless(listItems(database->fileInfoList) == 2, "Invalid file info list");
	fail_unless(listItems(database->dataList) == 0, "Invalid Data list");
	fail_unless(memcmp(str, data->data, data->len) == 0, "Invalid data from get");
	fail_unless(txn->offset == 0, "Invalid transaction offset");
	fail_unless(((FileInfo)(txn->fileHandle))->index == 0, "Invalid index from transaction file info");
	fail_unless(txn->dbHandle == dbEnv->dbHandle, "Invalid db handle in transaction");
	fail_unless(((FileInfo)(txn->fileHandle))->numRecords == 1, "Invalid number of records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->numActiveRecords == 1, "Invalid number of active records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->fd != -1, "Invalid file poiinter in transaction file info");


	free(data->data);
	free(data);
	free(txn);

	
	ret = DbGet(dbEnv, &data, &txn, &error);
	fail_unless(ret == 0, "DbGet failed");

	fail_unless(database->readOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid read offset");
	fail_unless(database->writeOffset == (1+LENGTH_SIZE+data->len+strlen(RECORD_DELIM)), "Invalid write offset");
	fail_unless(database->fileInfoCurrent == ListGetNext(ListGetNext(database->fileInfoList)), "Invalid file info current");
	fail_unless(listItems(database->fileInfoList) == 2, "Invalid file info list");
	fail_unless(listItems(database->dataList) == 0, "Invalid Data list");
	fail_unless(memcmp(str, data->data, data->len) == 0, "Invalid data from get");
	fail_unless(txn->offset == 0, "Invalid transaction offset");
	fail_unless(((FileInfo)(txn->fileHandle))->index == 1, "Invalid index from transaction file info");
	fail_unless(txn->dbHandle == dbEnv->dbHandle, "Invalid db handle in transaction");
	fail_unless(((FileInfo)(txn->fileHandle))->numRecords == 1, "Invalid number of records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->numActiveRecords == 1, "Invalid number of active records in transaction file info");
	fail_unless(((FileInfo)(txn->fileHandle))->fd != -1, "Invalid file poiinter in transaction file info");


	
	free(data->data);
	free(data);
	free(txn);

	ret = DbClose(dbEnv, &error);
	fail_unless(ret == 0, "DbClose failed");

	database = (Db)dbEnv->dbHandle;

	fail_unless(database == NULL, "Invalid database handle after closing database");




}
END_TEST


START_TEST(test_DbInit){
	DbEnv env;
	int retVal;
	int error;
	DbTxn txn[NUM_RECORDS];
	int explen;
        char* expdata;
	char* retData;
	Data data;
	Db database;
	FileInfo item;

	char buf[MAX_PATH_LEN];

	int i,j;

	//Set env parameters
	env = (DbEnv) malloc(sizeof(DbEnvStruct));
	env->databaseName = (char*)malloc(100);
	env->pathName = (char*)malloc(100);
	strcpy(env->databaseName, "dbInit");
	getcwd(buf, MAX_PATH_LEN);
        strcpy(&buf[strlen(buf)],TESTDBSDIR); 
	strcpy(env->pathName, buf);
       	env->numRecords = 1000;
	env->queueSize = 100;
	env->initialFileSize = 1024*1024;
	
	printf("Calling dbinit\n");
	//Initialize database
	retVal = DbInit(env, &error);
	fail_unless(retVal == 0, "DbInit failed");
	printf("After dbinit\n");
	
	database = (Db) env->dbHandle;
	for (i=0; i< listItems(database->fileInfoList); i++)
	  {
	    item=  (FileInfo)ListGetItem(database->fileInfoList,i);
	    if (item != NULL)
	      {
		printf("tail file index %d\n",item->index);
	       }
	    
	  }
	//Call dbput
	
	for (j=0; j<10; j++)
	  {
	data = (Data) malloc(sizeof(DataStruct));
	data->data = (char*) malloc(80);
	for(i = 0; i < NUM_RECORDS; ++i){
		sprintf(data->data, "induslogic%d", i);
		data->len = strlen(data->data);
		retVal = DbPut(env, data, &error);
		
	}
	free(data->data);
	free(data);
       	expdata = (char *) malloc(80);
	retData = (char *) malloc(80);
       	for ( i=0; i< NUM_RECORDS; ++i)
	  {
	    sprintf(expdata, "induslogic%d", i);
	    explen = strlen(expdata);
	    retVal = DbGet(env, &data, &txn[i], &error);
            strncpy(retData, data->data, explen);
	    retData[explen]='\0';
	    //	    printf("Data found %s for count %d\n", retData, i);
	    //printf("Data len %d expected len %d \n", data->len, explen);
	    /*if (txn[i] != NULL)
	      {
	      printf("Transaction offset %d\n", txn[i]->offset);
	    printf("Transaction file index %d\n", ((FileInfo)txn[i]->fileHandle)->index);
	    }
	      else
	    {
	      printf("Transaction is Null\n");
	    }
	    printf("Return value %d\n", retVal);*/
	    fail_unless(retVal ==0 && strcmp(expdata,retData) ==0 && explen ==data->len ,"DBGet failed ");
	    free(data->data);
	    free(data);

	    
	  }
	free(retData);
	  free(expdata);
	
	for ( i=0; i< NUM_RECORDS; ++i)
	  {
	    retVal =DbCommit(txn[i], &error);
	    fail_unless(retVal==0, "DBCommit failed");
	    free( txn[i]);
	  }
	  }

	//Close database
		DbClose(env, &error);
		fail_unless(env->dbHandle == 0, "Database not closed properly");
}
END_TEST
void initEnv()
{
	char buf[MAX_PATH_LEN];
	env = (DbEnv) malloc(sizeof(DbEnvStruct));
	env->databaseName = (char*)malloc(100);
	env->pathName = (char*)malloc(100);
	strcpy(env->databaseName, "testdbput");
	getcwd(buf, MAX_PATH_LEN);
        strcpy(&buf[strlen(buf)],TESTDBSDIR); 
	strcpy(env->pathName, buf);
	env->numRecords = MAX_RECORDS_FILE;
	env->queueSize = 7;
	env->initialFileSize = 1024*1024;
	setupDb(&database, env);
	
}
void setupDb(Db* dbptr, DbEnv env)
{
  Db database;
  FileInfo item;
  char buf[MAX_PATH_LEN];
  char fileName[200];
  int fd;

  // creat db dir
  sprintf(buf,"%s/%s",env->pathName, env->databaseName);
  mkdir( buf,S_IRWXU);
  sprintf(fileName, "%s/%s/%s.%d", env->pathName,env->databaseName, env->databaseName,0);

  *dbptr = (Db) malloc( sizeof(DbStruct));
  database = *dbptr;
  env->dbHandle = database;
  database->env = env;	
  database->fileInfoList = listInit();
  item = (FileInfo) malloc(sizeof(FileInfoStruct));
  item->index =0;
  item->numRecords =0;
  item->numActiveRecords =0;
  item->fd=open(fileName,O_CREAT|O_RDWR|O_SYNC,S_IRWXU);
  listAddItem(database->fileInfoList, (void *) item);
  database->fileInfoCurrent = ListGetNext(database->fileInfoList);
  database->writeOffset =  0;
  pthread_mutex_init(&database->dataMutex, NULL);
  pthread_mutex_init(&database->fileInfoMutex, NULL);



}
void removeDb()
{
  DbEnv env;
  int   i;
  char fileName[200];
  FileInfo finfo;

  env = database->env;
  for (i=0;i<=listItems(database->fileInfoList);i++)
    {
      finfo = (FileInfo) ListGetItem( database->fileInfoList, i);
      if ( finfo != NULL)
	{
      sprintf(fileName, "%s/%s/%s.%d", env->pathName,env->databaseName, env->databaseName,finfo->index);	  
      close(finfo->fd);
      //delete file and directory
      remove(fileName);
	}
      
    }
  listDestroy(database->fileInfoList);
  remove(env->pathName);
  free(env->databaseName);
  free(env->pathName);
  free(env);
  free(database);

}
START_TEST(DbPut_EmptyDb){
	DataStruct dt;
	FileInfo finfo;
	int i,error, retVal;

	//        initEnv(&env,2);
	// populate database structure
	//emptyDb(&database, env);
	dt.data = (char *) malloc(80);
	strcpy(dt.data, "test db put data");
	dt.len  = 80;
	
	retVal = DbPut( env, &dt, &error);

	fail_unless(retVal == 0, "DbPut failed for empty database");
	
	finfo =(FileInfo) listGetEndItem(database->fileInfoList);

	fail_unless(finfo != NULL && finfo->index == 0,"Tail file info not correctly added");
	fail_unless(finfo->numRecords == 1 && finfo->numActiveRecords == 1,"Tail file info num records incorrect");

	retVal = close(finfo->fd);
	
	fail_unless( retVal ==0 || (retVal ==-1 && errno != EBADF), "Bad file descriptor for tail file");

	free(dt.data);
	//removeDb(database);

}
END_TEST
START_TEST(DbPut_keep_current_open){
	DataStruct dt;
	FileInfo finfo;
	List     listPtr;
	char fileName[200];
	int i,error, retVal;
	int maxRecords =40;

	//        initEnv(&env, maxRecords);
	// populate database structure
	//emptyDb(&database, env);
	dt.data = (char *) malloc(80);
	strcpy(dt.data, "test db put data");
	dt.len  = 80;
	// put more than maximum records for a file
	for ( i= 0; i<=maxRecords; i++)
	  {
	    retVal = DbPut( env, &dt, &error);
	    fail_unless(retVal==0, "DbPut failed ");

	  }
	// expected results : two files have been created
	listPtr=ListGetNext(database->fileInfoList);
	while( listPtr != NULL)
	  {
	    finfo = (FileInfo) listPtr->item;
	    if ( finfo !=NULL)
	      {
		fail_unless(finfo->index ==0 
			       || finfo->index == 1,
			    "File number check failed" );
		// file descs for the files should be valid
		retVal = close( finfo->fd);
		fail_unless(retVal ==0 || errno != EBADF , "Bad file descriptor for open files");
		
	      }
	    	listPtr=ListGetNext(listPtr);
	  }

	free(dt.data);
	//removeDb(database);
}
END_TEST

START_TEST(DbPut_Close_old_file){
	DataStruct dt;
	FileInfo finfo;
	List     listPtr;
	char fileName[200];
	int i,error, retVal;
	int maxRecords =40;
	int numFiles =100;

	dt.data = (char *) malloc(80);
	strcpy(dt.data, "test db put data");
	dt.len  = 80;
	// put more than maximum records for a file
	for ( i= 0; i<=100*env->numRecords; i++)
	  {
	    retVal = DbPut( env, &dt, &error);
	    fail_unless(retVal==0, "DbPut failed ");

	  }
	// expected results : 101 files have been created from 0..100
	listPtr=ListGetNext(database->fileInfoList);
	while( listPtr != NULL)
	  {
	    finfo = (FileInfo) listPtr->item;
	    if ( finfo !=NULL)
	      {

		fail_unless(finfo->index >= 0 && finfo->index <=numFiles, "File number check failed" );
		// file descs for the file 0,2  should be valid
		retVal = close( finfo->fd);
		if ( finfo->index == 0 || finfo->index == numFiles )
		  {
		    fail_unless( retVal==0 || errno != EBADF, "Bad file descriptor for current and tail file");
		  }
		else
		  {
		    fail_unless(retVal != 0 && errno == EBADF, "File descriptor for intermediate files not closed");
		  }

	      }
	    	listPtr=ListGetNext(listPtr);
	  }

	free(dt.data);
	//removeDb(database);
}
END_TEST

void threadFunc(int* numputs)
{
  uint32_t i=0; 
  int retVal=0, error=0;
  DataStruct dt;

  dt.data = (char *) malloc(80);
  strcpy(dt.data, "test db put data");
  dt.len  = 80;
  

  for(i=0; i< *numputs; i++)
    {

     retVal = DbPut( env, &dt, &error);

    }
  pthread_exit(0);
}


START_TEST(DbPut_Multiple_Threads){
  int i;
  pthread_t pthread[NUM_PUT_THREADS];
  uint32_t numputs = 100, expFileNum, numTailRecs=0, expTailRecs=0;
  pthread_attr_t attr;

  pthread_attr_init(&attr);	
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
  pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
  for ( i=0; i< NUM_PUT_THREADS; i++)
    {
      pthread_create(&pthread[i], &attr,threadFunc, &numputs ); 
    }

   for ( i=0; i< NUM_PUT_THREADS; i++)
    {
      pthread_join(pthread[i], NULL); 
      }

   // assertions 
   expFileNum = (numputs*NUM_PUT_THREADS)/env->numRecords;
   if (  (expTailRecs=(numputs*NUM_PUT_THREADS)%env->numRecords) != 0)
     {
       expFileNum++;
     }
   else
     {
       expTailRecs =  env->numRecords;
     }


   fail_unless(expFileNum == listItems(database->fileInfoList), "Incorrect number of file info entries ");
   
   numTailRecs = ((FileInfo) listGetEndItem(database->fileInfoList))->numActiveRecords;
   fail_unless( expTailRecs == numTailRecs, "Incorrect number of active records for tail file");
  
}
END_TEST

void writeData( void* id)
{
  int threadId;
  int i, retVal;
  int error;
  uint32_t numRecs;
  uint32_t dataNum;
  DataStruct dt;
  char buf[20];
  
  threadId = (int) id;
  if (threadId == 0)
    {
      numRecs =    NUM_MT_RECORDS/NUM_WRITER_THREADS +    NUM_MT_RECORDS%NUM_WRITER_THREADS;
    
    }
  else
    {
      numRecs = NUM_MT_RECORDS/NUM_WRITER_THREADS;
    }

  for ( i= 0; i< numRecs; i++)
  {
    pthread_mutex_lock(&writeDataLock);
    dataNum = writeDataNum++;
    pthread_mutex_unlock(&writeDataLock);
    sprintf( buf, "%" PRId32 , dataNum);

    dt.data = buf;
    dt.len  = strlen(buf);
    retVal = DbPut( env, &dt, &error);
  }
  
}

void readData(void* id)
{
  int threadId;
  int i, retVal;
  int error;
  uint32_t recordsRead=0;
  uint32_t numRecs;
  uint32_t index;
  DbTxn txn;
  Data dt;
  char buf[20];



  threadId = (int) id;
  if (threadId == 0)
    {
      numRecs =    NUM_MT_RECORDS/NUM_READER_THREADS +    NUM_MT_RECORDS%NUM_WRITER_THREADS;
    
    }
  else
    {
      numRecs = NUM_MT_RECORDS/NUM_READER_THREADS;
    }

  while( recordsRead < numRecs)
    {

	retVal =DbGet(env, &dt, &txn, &error);


	if (retVal == 0 && dt !=NULL && dt->data !=NULL)
	  {
	    recordsRead++;
	    strncpy(buf, dt->data, dt->len);
	    buf[dt->len] ='\0';
	    sscanf(buf, "%"SCNd32, &index);
	    readDt[index] = readDt[index]+1;
	    //printf("Read data %"PRId32" %"PRId32 " %d\n",index, txn->offset,((FileInfo) txn->fileHandle)->index);


	    retVal=DbCommit(txn,&error);
	    if (retVal !=0 )
	      {
		printf("Dbcommit failed\n");
	      }
	    free(txn);
	    free(dt->data);
	    free(dt);
	    
	  }else{
	    printf("Dbget failed\n");
	  }
    }

}
START_TEST(MT_Quedb){
  pthread_t writer[NUM_WRITER_THREADS];
  pthread_t reader[NUM_READER_THREADS];
  char buf[MAX_PATH_LEN];
  int retVal,error;
  pthread_attr_t attr;
  uint32_t i;

  pthread_attr_init(&attr);	
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
  pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
  
  //  readData =(int *) malloc( NUM_MT_RECORDS);
  
  for(i=0; i< NUM_MT_RECORDS; i++)
    {
      readDt[i]=0;
    }

  env = (DbEnv) malloc(sizeof(DbEnvStruct));
  env->databaseName = (char*)malloc(100);
  env->pathName = (char*)malloc(100);
  strcpy(env->databaseName, "MTqueuedb");
  getcwd(buf, MAX_PATH_LEN);
  strcpy(&buf[strlen(buf)],TESTDBSDIR); 
  strcpy(env->pathName, buf);
  env->numRecords = 1000;
  env->queueSize = 100;
  env->initialFileSize = 1024*1024;
  
  printf("Calling dbinit\n");
  //Initialize database
  retVal = DbInit(env, &error);
  fail_unless(retVal == 0, "DbInit failed");
  printf("After dbinit\n");
  
  database = (Db) env->dbHandle;
  //creat writer and reader threads
  for ( i=0; i< NUM_WRITER_THREADS; i++)
    {
      pthread_create(&writer[i], &attr,writeData, i ); 
    }

  for ( i=0; i< NUM_READER_THREADS; i++)
    {
      pthread_create(&reader[i], &attr,readData, i ); 
    }
   for ( i=0; i< NUM_WRITER_THREADS; i++)
    {
      pthread_join(writer[i], NULL); 
      }


   for ( i=0; i< NUM_READER_THREADS; i++)
     {
      pthread_join(reader[i], NULL); 
     }

   
   printf("Data written %ld\n",writeDataNum );
   for(i=0; i< NUM_MT_RECORDS; i++)
    {
      if( readDt[i] != 1)
	printf("Data %ld read %d\n",i, readDt[i]);
    }

   for(i=0; i< NUM_MT_RECORDS; i++)
    {
      
      fail_unless(readDt[i] == 1, "Data not read");
    }
  //Close database
  DbClose(env, &error);
  fail_unless(env->dbHandle == 0, "Database not closed properly");

}
END_TEST

START_TEST(MT_Quedb_Read_First){
  pthread_t writer[NUM_WRITER_THREADS];
  pthread_t reader[NUM_READER_THREADS];
  char buf[MAX_PATH_LEN];
  int retVal,error;
  pthread_attr_t attr;
  uint32_t i;

  pthread_attr_init(&attr);	
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
  pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
  
  //  readData =(int *) malloc( NUM_MT_RECORDS);
  
  for(i=0; i< NUM_MT_RECORDS; i++)
    {
      readDt[i]=0;
    }

  env = (DbEnv) malloc(sizeof(DbEnvStruct));
  env->databaseName = (char*)malloc(100);
  env->pathName = (char*)malloc(100);
  strcpy(env->databaseName, "MTrfqueuedb");
  getcwd(buf, MAX_PATH_LEN);
  strcpy(&buf[strlen(buf)],TESTDBSDIR); 
  strcpy(env->pathName, buf);
  env->numRecords = 1000;
  env->queueSize = 100;
  env->initialFileSize = 1024*1024;
  
  printf("Calling dbinit\n");
  //Initialize database
  retVal = DbInit(env, &error);
  fail_unless(retVal == 0, "DbInit failed");
  printf("After dbinit\n");
  
  database = (Db) env->dbHandle;
  //creat writer and reader threads
  for ( i=0; i< NUM_READER_THREADS; i++)
    {
      pthread_create(&reader[i], &attr,readData, i ); 
    }
  
  for ( i=0; i< NUM_WRITER_THREADS; i++)
    {
      pthread_create(&writer[i], &attr,writeData, i ); 
    }

   for ( i=0; i< NUM_WRITER_THREADS; i++)
    {
      pthread_join(writer[i], NULL); 
      }

   for ( i=0; i< NUM_READER_THREADS; i++)
     {
      pthread_join(reader[i], NULL); 
     }

   
   printf("Data written %ld\n",writeDataNum );
  for(i=0; i< NUM_MT_RECORDS; i++)
    {
      if( readDt[i] != 1)
	printf("Data %ld read %d\n",i, readDt[i]);
    }

  for(i=0; i< NUM_MT_RECORDS; i++)
    {
      
      fail_unless(readDt[i] == 1, "Data not read");
    }
  //Close database
  DbClose(env, &error);
  fail_unless(env->dbHandle == 0, "Database not closed properly");

}
END_TEST

static void
run_suite()
{
	s = suite_create("QUEDB");
	tc1 = tcase_create("Database Manager");
	tc2 = tcase_create("DbPut");
	tc3 = tcase_create("Queue");
	sr = srunner_create(s);
	suite_add_tcase(s, tc1);
	tcase_add_test(tc1, test_DbInit);
	tcase_add_test(tc1, test_NewDb);
	tcase_add_test(tc1, test_putGet);
	tcase_add_test(tc1, test_singleRecordFile);
	
      
       	suite_add_tcase(s, tc2);
	tcase_add_checked_fixture(tc2, initEnv, removeDb);
	tcase_add_test(tc2, DbPut_EmptyDb);
	tcase_add_test(tc2, DbPut_Close_old_file);
	tcase_add_test(tc2,DbPut_keep_current_open);
	tcase_add_test(tc2,DbPut_Multiple_Threads);
	suite_add_tcase(s, tc3);
       	tcase_add_test(tc3,MT_Quedb);
	tcase_add_test(tc3,MT_Quedb_Read_First);
	
	srunner_run_all(sr, CK_VERBOSE);
	srunner_free(sr);
	suite_free(s);
}




int main(void)
{
	run_suite();
	return(0);
}
