#include "queue.h"
#include "datamanager.h"
#include <string.h>

#include <unistd.h>

/*
 * Get last inserted record from the queueDB
 * Read record from disk, if record is not present
 * in memory queue
 */
int
DbGet(DbEnv env, Data *data, DbTxn* t, int* error)
{
	Db database;
	DataInfo current;
	char *fn = "DbGet";
	int ret;
	
	//Get database pointer from environment
	database = (Db)env->dbHandle;
	
	//Lock data info list
	pthread_mutex_lock(&database->dataMutex);
	
	//If no items in data info list
	if(listItems(database->dataList) == 0){


		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No records in data info list, populating data info list\n", fn));

		//Lock file info list
		pthread_mutex_lock(&database->fileInfoMutex);

		ret = PopulateDataInfoList(database, error);

		//Unlock file info mutex
		pthread_mutex_unlock(&database->fileInfoMutex);
		
		//No record in Data info list. Populate data info list from data files
		if(ret < 0){

			//No records in database
			//Return NULL
			NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No records in database, returning NULL\n", fn));

			
			*data = NULL;
			*t = NULL;

			//Unlock data info mutex
			pthread_mutex_unlock(&database->dataMutex);
			
			return(-1);
		}

	}


			
	//Get first data from data info list
	current = (DataInfo)listDeleteFirstItem(database->dataList);
	
	//Unlock data info mutex
	pthread_mutex_unlock(&database->dataMutex);
	
	if(current == NULL){
		//No records in database
		//Return NULL
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No records in database, returning NULL\n", fn));
		
		NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s No records in database, returning NULL\n", fn));
		*data = NULL;
		*t = NULL;
			
		return(-1);
	}
		



	//Allocate memory for DbTxn which user will have to free later
	*t = (DbTxn) malloc(sizeof(DbTxnStruct));	

	//Copy data
	*data = current->data;


	//Create transaction identifier
	(*t)->offset = current->offset;
	(*t)->fileHandle = (uintptr_t) current->fileInfo;
	(*t)->dbHandle = env->dbHandle;

	//Free the data info node
	free(current);

	//Delete list item
	return(0);	
}

