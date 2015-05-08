#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <pthread.h>
#include <stdio.h>
#include "queDB.h"
#include <stdlib.h>
#include "srvrlog.h"

#define MAX_ERR_LEN 200
#define TRUE 1
#define FALSE 0
#define MAX_PATH_LEN 100
#define MAX_STRING_LEN 80
#define STATUS_ACTIVE_RECORD 0
#define STATUS_DELETED_RECORD 1
#define RECORD_STATUS_SIZE 1
#define LENGTH_SIZE 10
#define MAX_DATA_LENGTH 500
#define RECORD_DELIM "\n"

//Error values
#define SUCCESS 0
#define ERR_NO_DATA_FILE 2000
#define ERR_NO_DATA_IN_CURRENT_FILE 2001
#define ERR_DATA_CORRUPTED 2002
#define ERR_ALLOCATING_MEM 2003
#define ERR_WRITING_FILE 2004
#define ERR_GET_NAME_FROM_INDEX 2005
#define ERR_OPENING_FILE 2006
#define ERR_DELETING_FILE 2007
#define ERR_FILE_PATH 2008
#define ERR_READING_FILE 2009
#define ERR_DELETING_MQUE 2010
#define ERR_ADDING_MQUE 2011
#define ERR_CREATING_DIR 2012
#define ERR_OPENING_DIR 2012


typedef struct _db_
{
	DbEnv env; //Environment of database
	List dataList;	//Data list 
	List fileInfoList;	//File info list
        List fileInfoCurrent;// Current position in fileInfoList
	uint32_t readOffset;	//Offset where data is to be read
        uint32_t writeOffset;	//Offset where data is to be written
	pthread_mutex_t dataMutex;
	pthread_mutex_t fileInfoMutex;
}DbStruct, *Db;


typedef struct _fileinfo_
{
	int index;	//Index of data file which will translate to file name
	uint32_t numRecords;	//Number of records in file
	uint32_t numActiveRecords;	//Number of active records in file
	int fd;	//File Descriptor of corrosponding file
}FileInfoStruct, *FileInfo;

typedef struct _datainfo_
{
	uint32_t offset;  //Data file offset from where data is read
	FileInfo fileInfo;//File Info to file which data belongs
	Data data; //Data
}DataInfoStruct, *DataInfo;




#endif
