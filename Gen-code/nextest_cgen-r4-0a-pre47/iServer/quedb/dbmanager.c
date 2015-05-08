#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "queue.h"
#include "utils.h"
#include "metadatamanager.h"
#include "datamanager.h"
#include <string.h>
#include <dirent.h>

static int GetDatabase(DbEnv env, int *error);
static int IsNewDb(DbEnv env);
static int CreateNewDb(DbEnv env, int *error);
static int AllocDb(DbEnv env, Db* database, int *error);

/*
 * Initialize database and return
 * environment handle to the caller
 */ 
int 
DbInit(DbEnv env, int* error)
{
	char *fn = "DbInit";
	
	//If new database
	if(IsNewDb(env)){

		
		//Create a new database
		if(CreateNewDb(env, error) != 0){
			
			NETERROR(MQUEDB, ("%s Error creating new database\n",fn));
		return(-1);
		}
	}

	//Get database initializing data info and file info lists
	if(GetDatabase(env, error) != 0){
		
		NETERROR(MQUEDB, ("%s Error populating database information\n",fn));
		return(-1);
	}
	
	return(0);
}


/*
 * Check whether database is 
 * already existing or not
 */
static int
IsNewDb(DbEnv env)
{
	int fileHandle;
	char openPath[MAX_PATH_LEN];
	char *fn = "IsNewDb";
	DIR *dirDesc;
	struct dirent *dirFiles;
	int count = 0;
	
	//Get data file path in database
	if(GetFilePath(env, openPath) != 0){
		NETERROR(MQUEDB, ("%s Error getting data file path in database", fn));
		return(-1);
	}
	
	//Check whether database exists
	fileHandle = open(openPath, O_CREAT|O_EXCL);
	
	//If database exists
	if(errno == EEXIST){
		
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s Database already exists", fn));
		close(fileHandle);
		if((dirDesc = opendir(openPath)) == NULL){
			NETERROR(MQUEDB, ("%s Error opening database directory\n", fn));
			remove(openPath);
			return(-1);
		}
		while((dirFiles = readdir(dirDesc)) != NULL){
			count++;
		}
		closedir(dirDesc);
		//No files in database
		if(count == 2){
			remove(openPath);
			return(TRUE);
		}
		
		//Database exists with datafiles more than zero
		return(FALSE);
	}
	else{
		
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s New database", fn));
		
		//New database
		close(fileHandle);
		
		//Remove regular file created due to test
		remove(openPath);
		
		return(TRUE);
	}
}

/*
 * Create new blank database
 * with metadata files and data file
 */
static int
CreateNewDb(DbEnv env, int *error)
{
	char path[MAX_PATH_LEN];
	char *fn = "CreateNewDB";
	int fileDesc;
	int firstFileIndex = 0;

	//Get data file path for the database
	if(GetFilePath(env, path) != 0){
		NETERROR(MQUEDB, ("%s Error getting data file path in database", fn));
		*error = ERR_FILE_PATH;
		return(-1);
	}
		
	//Create database directory
	if(mkdir(path, S_IRWXU) != 0){
		
		NETERROR(MQUEDB, ("%s: Error creating directory: %d\n",fn, errno));
		*error = ERR_CREATING_DIR;
		return(-1);
	}
	
	//Add initial data file
	if(OpenDataFile(env, firstFileIndex, &fileDesc, error) < 0){
		
		NETERROR(MQUEDB, ("%s: Error creating data file\n",fn));
		return(-1);
	}
	
	//Close data file descriptor
	close(fileDesc);
	
	*error = SUCCESS;
	return(0);
}

/*
 * Get database and read records from the
 * database populating database structures.
 */
static int
GetDatabase(DbEnv env, int *error)
{
	Db database;
	char* fn = "GetDatabase";

	if(AllocDb(env, &database, error) < 0){
		NETERROR(MQUEDB, ("%s Error while allocating database\n", fn));
		*error = ERR_ALLOCATING_MEM;
		return(-1);
	}

	//Get fileInfo list
	if(GetFileInfoList(database, error) < 0){
		NETERROR(MQUEDB, ("%s Error while getting file info list\n", fn));
		return(-1);
	}
	
	//Get data list and update read offset
	if(InitDataInfoList(database, error) < 0){
		NETERROR(MQUEDB, ("%s Error while getting data list\n", fn));
		return(-1);
	}
	
	if(GetWriteOffset(database, error) < 0){
		NETERROR(MQUEDB, ("%s: Error while getting write offset\n", fn));
		return(-1);
	}

	//Store database pointer in environement	
	env->dbHandle = (uintptr_t) database;

	*error = SUCCESS;
	return(0);	
}

/*
 * Allocate database structure
 */
static int
AllocDb(DbEnv env, Db* database, int* error){
	
	//Allocate memory for database
	*database = (Db) malloc(sizeof(DbStruct));

	//Store environment in database
	(*database)->env = env;

	//Initialise mutex
	pthread_mutex_init(&(*database)->dataMutex, NULL);
	pthread_mutex_init(&(*database)->fileInfoMutex, NULL);
	
	*error = SUCCESS;
	return(0);
}

/*
 * Close database and release
 * all the resources
 */
int
DbClose(DbEnv env, int* error){

	Db database = (Db) env->dbHandle;

	//Free Items in data list
	FreeDataInfoListItems(database->dataList, error);

	//Destroy data list
	listDestroy(database->dataList);

	//Free Items in file info list
	FreeFileInfoListItems(database->fileInfoList, error);

	//Destroy file info list
	listDestroy(database->fileInfoList);

	//Destroy mutex
	pthread_mutex_destroy(&database->dataMutex);
	pthread_mutex_destroy(&database->fileInfoMutex);

	free(database);
	env->dbHandle = (uintptr_t)0;

	*error = SUCCESS;
	return(0);
}
