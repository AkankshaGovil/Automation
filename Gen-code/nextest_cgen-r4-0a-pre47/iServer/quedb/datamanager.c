#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "datamanager.h"
#include "metadatamanager.h"
#include "queue.h"
#include "utils.h"
#include "list.h"
#include <errno.h>



static int AllocDataInfo(DataInfo *item, int *error);

/*
 * Read record from given data file
 * updating file offset
 * also, allocating the DataInfo node, 
 * inserting data into it
 */
int
ReadRecord(DbEnv env, FileInfo fileInfo, uint32_t *readOffset, DataInfo *dataInfo, int* error)
{
	char *fn = "ReadRecord";
 	char flagStr[2];
	int flag;
	char len[LENGTH_SIZE];
	uint32_t length;
	ssize_t ret;
	int fd;
	
	
	fd = fileInfo->fd;
        lseek(fd, (off_t)*readOffset, SEEK_SET);

	ret = read(fd, (void*)&flagStr, 1);

	if(ret <= 0 || flagStr[0] == '\0'){
        //No data to be read
	NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No data in current data file to be read\n", fn));
	*dataInfo = NULL;
	*error = ERR_NO_DATA_IN_CURRENT_FILE;
	return(-1);
	}
	
	while(ret > 0 && flagStr[0] != '\0'){
		
		flagStr[1]='\0';
		sscanf(flagStr, "%d", &flag);
		if(read(fd, (void*)len, LENGTH_SIZE) != LENGTH_SIZE){
			NETERROR(MQUEDB, ("%s Error reading file length, data corrupted in data file\n", fn));
			*dataInfo = NULL;
			*error = ERR_DATA_CORRUPTED;
			return(-1);
		}

		len[LENGTH_SIZE] = '\0';
		sscanf(len, "%"SCNu32, &length);
//		length = atoi(len);
  
		//Active record
		if(flag == STATUS_ACTIVE_RECORD){
			//Allocate memory for data
			if(AllocDataInfo(dataInfo, error) < 0){
				NETERROR(MQUEDB,  ("%s Error allocating memory for data\n", fn));
				*dataInfo = NULL;
				*error = ERR_ALLOCATING_MEM;
				return(-1);
			}
		
			//Populate data
			(*dataInfo)->data->data = (char*)malloc(length);
			read(fd, (void*)(*dataInfo)->data->data, length);
			(*dataInfo)->data->len = length;


		
			//Populate other data info attributes
			(*dataInfo)->fileInfo = fileInfo;
			(*dataInfo)->offset = *readOffset; 	
			
			*readOffset = *readOffset + 1 + LENGTH_SIZE + length;
			//If record delimiter is defined
			if(RECORD_DELIM != NULL){
				*readOffset = *readOffset + strlen(RECORD_DELIM);
			}
			*error = SUCCESS;
			return(0);
		}
		else{
			//Move file offset past the data
			lseek(fd, (off_t)(length), SEEK_CUR);
		
			//If record delimiter is defined
			if(RECORD_DELIM != NULL){
				lseek(fd, (off_t)(strlen(RECORD_DELIM)), SEEK_CUR);
			}
		
			*readOffset = *readOffset + 1 + LENGTH_SIZE + length;
		
			//If record delimiter is defined
			if(RECORD_DELIM != NULL){
				*readOffset = *readOffset + strlen(RECORD_DELIM);
			}
		}
  
		//Read flag for next record
		ret = read(fd, (void*)&flagStr, 1);
	}
	
	//All records in this file are deleted
	*dataInfo = NULL;
	return(-1);
	
}

/*
 * Write record to a given data file
 * updating file offset
 */
int
WriteRecord(DbEnv env, FileInfo fileInfo,Data dt, uint32_t* offset, int* error)
{
  static char fn[]="WriteRecord";
  char *buf;
  size_t buflen=0;
  ssize_t byteWritten = 0;
  size_t delimLength=0;
  size_t totalen=0;

  if ( RECORD_DELIM == NULL)
    {
      delimLength =0;
    }
  else
    {
      delimLength = strlen( RECORD_DELIM);
    }

  totalen =  LENGTH_SIZE + RECORD_STATUS_SIZE + dt->len + delimLength;
 
  buf = (char *) malloc(totalen);
 
    // seek to the desired offset
  lseek( fileInfo->fd, (off_t)*offset, SEEK_SET);
 
  snprintf( buf, RECORD_STATUS_SIZE +1 , "%i", STATUS_ACTIVE_RECORD );
  buflen += RECORD_STATUS_SIZE;

  GetPaddedString( dt->len, LENGTH_SIZE, &buf[buflen]);
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Padded string for data length %s\n ",fn,&buf[buflen]));
  buflen += LENGTH_SIZE;


  
  memcpy( &buf[buflen],dt->data, dt->len );
  buflen += dt->len;


		
  if ( delimLength !=0 )
    {
      strncpy( &buf[buflen],RECORD_DELIM, delimLength );
      buflen += delimLength;
    }


  //Initialize byte written as buflen
  byteWritten = buflen;

  // write record to the file
  if ( write( fileInfo->fd, buf, buflen) != byteWritten)
    {
      NETERROR( MQUEDB, ("%s Unable to write to file fd %d number %d errno %d\n", fn, fileInfo->fd, fileInfo->index,errno));
      free(buf);
      *error = ERR_WRITING_FILE;
      return -1;
    }

  // increment the active record count for this file
  fileInfo->numRecords++;
  fileInfo->numActiveRecords++;
  *offset += buflen;
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s File %d active %"PRIu32" total %"PRIu32" \n",fn, fileInfo->index, fileInfo->numActiveRecords, fileInfo->numRecords));
  free(buf);
  *error = SUCCESS;
  return 0;
}

/*
 * Mark record as deleted in given data file
 */
int 
DeleteRecord(DbEnv env, FileInfo fileInfo,uint32_t offset, int* error)
{
  char recordStatus[2];
  static char fn[] = "DeleteRecord";

  // seek to the desired offset
  lseek( fileInfo->fd, (off_t) offset, SEEK_SET);

  snprintf( recordStatus, sizeof(recordStatus), "%i", STATUS_DELETED_RECORD );

  // update record status to deleted

  if ( write( fileInfo->fd, recordStatus, sizeof(recordStatus)-1) == -1)
    {
      NETERROR(MQUEDB,("%s Error writing to file %d for update : errno %d\n", fn,fileInfo->index, errno));
      *error = ERR_WRITING_FILE;
      return -1;
    }
  // decrement the active record count for this file
  fileInfo->numActiveRecords--;
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s File %d active %"PRIu32" total %"PRIu32" \n",fn, fileInfo->index, fileInfo->numActiveRecords, fileInfo->numRecords));

  *error = SUCCESS;
  return 0;
  
}

/*
 * Create a data file provided it's path
 * and file name
 */
int
OpenDataFile(DbEnv env, int fileIndex, int *fileDesc, int* error){
	char filePath[MAX_PATH_LEN];
	char fileName[MAX_PATH_LEN];
	char dbPath[MAX_PATH_LEN];
	char *fn = "CreateDataFile";
	static char initialData = '\0';
	int ret = 0;
	
	//Get data file path
	GetFilePath(env, dbPath);
	
	//Generate file name from index
	if(GetNameFromIndex(env, fileIndex, fileName, error) != 0){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Get Name from index failed", fn));
		*error = ERR_GET_NAME_FROM_INDEX;
		return(-1);
	}
	
	//Generate absolute path for the data file
	sprintf(filePath, "%s/%s", dbPath, fileName);
	
	//Open data file
	if((*fileDesc = open(filePath, O_CREAT|O_RDWR|O_SYNC,S_IRWXU)) < 0){
		NETERROR(MQUEDB, ("%s Error opening data file\n",fn));
		*error = ERR_OPENING_FILE;
		return(-1);
	}

	if(lseek(*fileDesc, 0, SEEK_END) == 0){
		lseek(*fileDesc, (off_t)env->initialFileSize, SEEK_SET);
		if((ret = write(*fileDesc, &initialData, 1)) != 1){
			NETERROR(MQUEDB, ("%s Error writing initial null data\n", fn));
			*error = ERR_WRITING_FILE;
			return(-1);
		}
	}
	
	lseek(*fileDesc, 0, SEEK_SET);
	
	*error = SUCCESS;
	return(0);
}

/*
 Delete the data file with the file index 
 from the filesystem
*/	
int 
DeleteDataFile(DbEnv env, int fileIndex, int* error)
{
  char filePath[MAX_PATH_LEN];
  char fileName[MAX_PATH_LEN];
  char dbPath[MAX_PATH_LEN];
  char *fn = "DeleteDataFile";

  GetFilePath(env, dbPath);
  GetNameFromIndex(env, fileIndex, fileName, error);
  sprintf(filePath, "%s/%s", dbPath, fileName);
  
  if ( unlink( filePath ) == -1)
    {
      NETERROR(MQUEDB, ("%s:Error deleting data file : %s\n",fn, filePath));
      *error = ERR_DELETING_FILE;
      return (-1);
    }

  *error = SUCCESS;
  return (0);
}

/*
* Get data list and update 
* read offset, 
* number of records present in data queue
*/
int 
InitDataInfoList(Db database, int *error){
	char *fn = "InitDataInfoList";
	
	//Initialise data info list
	database->dataList = listInit();
	database->readOffset = 0;
	
	//Populate data info list from data files
	if(PopulateDataInfoList(database, error) == -1){
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No data in database\n", fn));
	}
	
	return(0);
}

/*
 * Function to populate data info list by reading records 
 * from the datafiles
 */
int
PopulateDataInfoList(Db database, int* error){
	DataInfo item;
	uint32_t count = 0;
	char *fn = "PopulateDataInfoList";
	int ret;
	FileInfo fileInfo;
	int active = 0;
	int lastOffset;
	
	//If no data files in database
	if(database->fileInfoCurrent == NULL){
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No data file in database", fn));
		*error = ERR_NO_DATA_FILE;
		return(-1);
	}

	//If no elements in file info list
	if(listItems(database->fileInfoList) == 0){
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No data in data files to be read\n", fn));
		*error = ERR_NO_DATA_FILE;
		return(-1);
	}
		
	//Get current file info from file info list
	fileInfo = (FileInfo)((List) database->fileInfoCurrent)->item;
	
	//File not open for read
	if(fileInfo->fd == -1){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Data file not open, opening\n", fn));
		OpenDataFile(database->env, fileInfo->index, &fileInfo->fd, error);
		//Get record count
		GetRecordCount(database->env, fileInfo->fd, &fileInfo->numRecords, &fileInfo->numActiveRecords, &lastOffset, error);
		database->readOffset = 0;
	}
		active = fileInfo->numActiveRecords;
	

	//Try populating number of data in data queue specified in environment
	while(count < database->env->queueSize){

	//No active records in current data file		
	if(active == 0){
		
		//No more data files
		if(database->fileInfoCurrent == NULL || database->fileInfoCurrent->next == NULL){
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No more data files in database\n", fn));
		*error = ERR_NO_DATA_FILE;
		return(0);
		}

		//Get next data file
		database->fileInfoCurrent = ListGetNext(database->fileInfoCurrent);
		fileInfo = (FileInfo)((List) database->fileInfoCurrent)->item;
		database->readOffset = 0;

	//File not open for read
	if(fileInfo->fd == -1){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Data file not open, opening\n", fn));
		OpenDataFile(database->env, fileInfo->index, &fileInfo->fd, error);
		//Get record count
		GetRecordCount(database->env, fileInfo->fd, &fileInfo->numRecords, &fileInfo->numActiveRecords, &lastOffset, error);
	}
		active = fileInfo->numActiveRecords;
	}
	else{

	//Read a record from the database
	ret = ReadRecord(database->env, (FileInfo)fileInfo, &database->readOffset, &item, error);
	active--;
	
	if(item == NULL){
		NETERROR(MQUEDB, ("%s Item returned from Read Record is null\n", fn));
	}
	else{
		
	//Add data in data info list
	listAddItem(database->dataList, (void*)item);
	
	//Increment count	
	count++;
	}
	}
      }


*error = SUCCESS;
return(0);
}

/*
 * Update write offset for the database
 */
int
GetWriteOffset(Db database, int *error)
{
	FileInfo tail;
	char dbPath[MAX_PATH_LEN];
	char *fn = "GetWriteOffset";
	
	//Get last data file
	tail = (FileInfo)listGetEndItem(database->fileInfoList);
	
	//Get data file path in database
	if(GetFilePath(database->env, dbPath) != 0){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Unable to get file path\n", fn));
		*error = ERR_FILE_PATH;
		return(-1);
	}

	//If data file not open
	if(tail->fd == -1){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s file not open, opening\n", fn));
		if(OpenDataFile(database->env, tail->index, &tail->fd, error) < 0){
			NETERROR(MQUEDB, ("%s:Error opening file to write data",fn));
			*error = ERR_OPENING_FILE;
			return(-1);
		}
	}

	//Update record count
	GetRecordCount(database->env, tail->fd, &tail->numRecords, &tail->numActiveRecords, &database->writeOffset, error);

	*error = SUCCESS;
	return(0);
}



/*
 * Determine number of records in a file
 */
int
GetRecordCount(DbEnv env, int fd, int *recordCount, int *activeRecordCount, uint32_t *writeOffset, int *error){
	char flagStr[2];
	int flag;
	uint32_t length;
	int ret;
	char len[LENGTH_SIZE];
	char *fn = "GetRecordCount";
	*recordCount = 0;
	*activeRecordCount = 0;	
	*writeOffset = 0;

	//Reset file descriptor to the beginning
	lseek(fd, 0, SEEK_SET);
	
	//Read flag for data
	ret = read(fd, (void*)flagStr, 1);
	
	//Read data file till next record exists
	while(ret != 0 && (flagStr[0] == '0' || flagStr[0]  == '1') && flagStr[0] != '\0'){


	//Read data length
	if(read(fd, (void*)len, LENGTH_SIZE) != LENGTH_SIZE){
		NETERROR(MQUEDB, ("%s Error reading file length\n", fn));
		*error = ERR_READING_FILE;
		return(-1);
	}

	flagStr[1] = '\0';
	len[LENGTH_SIZE] = '\0';
	

	
	//Convert data flag and data length from string to integer
	sscanf(len, "%"SCNu32, &length);
	sscanf(flagStr, "%d", &flag);

	//Increment record count
	(*recordCount)++;
	
	//Record not deleted, increment count
	//increment active record count
	if(flag == STATUS_ACTIVE_RECORD){
		(*activeRecordCount)++;
	}


	//Move file pointer past data
	*writeOffset = lseek(fd, (off_t)length, SEEK_CUR);

	//If record delimiter is not null, move pointer past delimiter
	if(RECORD_DELIM != NULL){
		*writeOffset = lseek(fd, strlen(RECORD_DELIM), SEEK_CUR);
	}
	
	
	//Read flag for next data
	ret = read(fd, (void*)flagStr, 1);
	}

	
	*error = SUCCESS;
	return(0);
}
		
		

/*
 * Allocate data info node
 */
static int
AllocDataInfo(DataInfo* item, int *error){

	//Allocate memory for data info
	*item = (DataInfo) malloc(sizeof(DataInfoStruct));

	//Allocate memory for data
	(*item)->data = (Data) malloc(sizeof(DataStruct));
	
	*error = SUCCESS;
	return(0);
}

/*
 * Free items in data info list
 */
int
FreeDataInfoListItems(List dataInfoList, int *error){
	DataInfo item;
	
	while(listItems(dataInfoList) != 0){
	
		item = (DataInfo) listDeleteFirstItem(dataInfoList);
		free(item->data->data);
		free(item->data);
		free(item);
	}
	
	*error = SUCCESS;
	return(0);
}
