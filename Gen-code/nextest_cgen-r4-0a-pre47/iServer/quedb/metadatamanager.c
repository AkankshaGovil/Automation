#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "datamanager.h"
#include "metadatamanager.h"
#include "utils.h"

int GetNextFileNo(Db database);
static int GetIndexFromName(char name[], int *index, int *error);

/*
 * Delete current file info
 * record 
 */
int
DeleteFileInfo(Db database, FileInfo fileInfo, int* error )
{
  static char fn[] ="DeleteFileInfo";
  List current =NULL;
  FileInfo currentFileInfo=NULL;
  FileInfo tail=NULL;

  current = database->fileInfoCurrent;
  if ( current != NULL)
    {
      currentFileInfo = (FileInfo) database->fileInfoCurrent->item;
    }
  tail    = (FileInfo) listGetEndItem( database->fileInfoList);

  if ( close( fileInfo->fd) == -1 )
    {
      NETERROR(MQUEDB, ("%s Error occured closing fd %d for file %d\n",fn, fileInfo->fd,fileInfo->index));
    }
  // delete the data file associated with the file index
  if (DeleteDataFile( database->env, fileInfo->index, error) ==-1 )
    {
      NETERROR(MQUEDB, ("%s Error occured deleting file %d from file system\n",fn, fileInfo->index));
        *error = ERR_DELETING_FILE;
	return(-1);
    }

  if ( currentFileInfo !=NULL && currentFileInfo->index == fileInfo->index )
    {
      //clear read offset
      database->readOffset =0;
      //move current to the next file
      database->fileInfoCurrent = ListGetNext(current);
    }
  
  if ( tail->index == fileInfo->index )
    {
      //clear write offset
      database->writeOffset =0;
    }
  
 // delete fileinfo  list entry
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Deleting fileInfo for %d from file info list\n",fn, fileInfo->index));

  if ( listDeleteItem ( database->fileInfoList, (void * ) fileInfo) == -1) 
    {
      NETERROR(MQUEDB, ("%s Error occured deleting file info from list\n",fn));
      *error = ERR_DELETING_MQUE;
      return (-1);
    }
 return (0);

}

/*
 * Add file info record
 * into fileInfo list.
 */
int
AddFileInfo(Db database, int* error)
{
  static char fn[]="AddFileInfo";
  int fileno;
  int fd; 
  FileInfo fileInfo;
  // get the next least positive unused number from the FileInfo list.
  fileno=GetNextFileNo( database);
      NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s File number to be created %d \n",fn,fileno));  
  
  // create data file with the number suffixed
  if ( OpenDataFile( database->env, fileno, &fd, error) == -1)
    {
      NETERROR(MQUEDB, ("%s Error occured opening file %d\n",fn,fileno));
      return (-1);
    }

  // initialize a new FileInfo record and insert at end.
  fileInfo = (FileInfo) malloc ( sizeof( FileInfoStruct ));
  if ( fileInfo == NULL )
    {
      //out of memory
      NETERROR(MQUEDB, ("%s Out of memory\n",fn));
      *error = ERR_ALLOCATING_MEM;
      return -1; 
    }

  fileInfo->fd =fd;
  fileInfo->numRecords = 0;
  fileInfo->numActiveRecords = 0;
  fileInfo->index = fileno;

  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Adding file info entry to list for file %d \n",fn, fileInfo->index));
  if ( listAddItem( database->fileInfoList, (void *) fileInfo) == -1)
    {
      NETERROR(MQUEDB,("%s Error Adding file info entry to list for file %d \n",fn, fileInfo->index));
      *error = ERR_ADDING_MQUE;
      return (-1);
    }
  
  // set current if required
  if (database->fileInfoCurrent == NULL)
    {
      database->fileInfoCurrent = ListGetNext(database->fileInfoList);
    }
  
  // set write offset to zero
  database->writeOffset = 0;  
  *error = SUCCESS;
  return(0);
}

/*
 * Populate file info list for the database
 * environment.
 */
int 
GetFileInfoList(Db database, int *error){
	char dbPath[MAX_PATH_LEN];
	char *fn = "GetFileInfoList";
	FileInfo node;
	DIR* dirDesc;
	struct dirent *dirFiles;
	int *index;
	int count = 0;
	int validCount = 0;
	int i;
	char buf[MAX_PATH_LEN];

	
	//Get data file path in database
	GetFilePath(database->env, dbPath);

	
	if((dirDesc = opendir(dbPath)) == NULL){
		NETERROR(MQUEDB, ("%s Error opening database directory\n", fn));
		*error = ERR_OPENING_DIR;
		return(-1);
	}

	//Get number of directory entries
	while((dirFiles = readdir(dirDesc)) != NULL){
		count++;
	}
	rewinddir(dirDesc);
	
		
	if(count == 0){
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No data files in database\n", fn));
		*error = ERR_NO_DATA_FILE;
		return(-1);
			
	}
	
	if(count != 0){
		
	index = (int*) malloc( count * sizeof(int));

	database->fileInfoList = listInit();
	
	while((dirFiles = readdir(dirDesc)) != NULL){
		
		strcpy(buf, dirFiles->d_name);
		//If directory name is valid
		if(IsValidName(database->env, buf)){
		 NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s Valid data file, generating index\n", fn));
		 strcpy(buf, dirFiles->d_name);
		 //Get file index from file name
		 if( GetIndexFromName(buf, &index[validCount++], error) != 0){
			NETERROR(MQUEDB, ("%s Get index from name failed\n",fn));
		 }
			 
		}
	}

	
	//Sort all the file index
	SortInt(index, validCount);
	
	for(i = 0; i < validCount; i++){
		//Initialise and populate file info node
		node = (FileInfo) malloc(sizeof(FileInfoStruct));
		node->index = index[i];
		node->fd = -1;
		node->numRecords = 0;
		node->numActiveRecords = 0;
		listAddItem(database->fileInfoList, node);	
	}
	}// end if count != 0
	database->fileInfoCurrent = (List)database->fileInfoList->head->begin->next;
//	database->fileInfoCurrent = ListGetNext(database->fileInfoList);

	closedir(dirDesc);


*error = SUCCESS;
return(0);
}

/*
 * Return next file index for the
 * file to be created.
 */
int
GetNextFileNo(Db database){
 FileInfo tailFileInfo;
 tailFileInfo = (FileInfo) listGetEndItem( database->fileInfoList);
 // increment the tail file number for the new file to be created
 if ( tailFileInfo != NULL)
   {
     return (tailFileInfo->index +1);
   }
 else
   {
     return 1;
   }


}

/*
 * Return file index from file name
 */
static int
GetIndexFromName(char name[], int *index, int *error){
	char *token;
	char *ptrptr;
	char *fn = "GetIndexFromName";
	strtok_r(name, ".", &ptrptr);
	token = strtok_r(NULL, ".", &ptrptr);
	NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Extracting index from name", fn));
	*index = atoi(token);

	*error = SUCCESS;
	return(0);
}

/*
 * Return file name from
 * file index
 */
int
GetNameFromIndex(DbEnv env, int index, char name[], int *error){
	char *fn = "GetNameFromIndex";
	NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Generating name from index", fn));
	//Generate file name using database name and file index
	sprintf(name, "%s.%d", env->databaseName, index);
	*error = SUCCESS;
	return(0);
}


/*
 * Check whether file name is valid for this database.
 */
int
IsValidName(DbEnv env, char name[]){
	char *tok;
	char *ptrptr;
	char *fn = "IsValidName";
	tok = strtok_r(name, ".", &ptrptr);
	if(tok == NULL){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s File entry does not contain seperator", fn));
		return(0);
	}
	//If database name is not same as file prefix
	if(strcmp(env->databaseName, tok)){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Database name is not same as data file prefix", fn));
		return(0);
	}
	tok = strtok_r(NULL, ".", &ptrptr);
	if(tok == NULL){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Data file does not contain file index", fn));
		return(0);
	}
	//If suffix does not consist of number only
	if(!IsNumInString(tok)){
		NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s file index consist of only numbers, valid data file", fn));
		return(0);
	}
	return(1);
}

/*
 * Free items in file info list
 */
int
FreeFileInfoListItems(List fileInfoList, int *error){
	FileInfo item;
	//Extract each entry from file info list
	while(listItems(fileInfoList) != 0){
		//Delete first item from file info node
		item = (FileInfo) listDeleteFirstItem(fileInfoList);
		//Close file descriptor
		close(item->fd);
		//Free item
		free(item);
	}
	*error = SUCCESS;
	return(0);
}
