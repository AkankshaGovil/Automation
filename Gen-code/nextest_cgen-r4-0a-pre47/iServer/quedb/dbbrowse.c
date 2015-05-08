#include "queue.h"
#include <unistd.h>
#include <string.h>

#define NUM_RECORDS 100
#define QUEUE_SIZE 100
#define FILE_SIZE 1024


static void GetDbPath(char path[], char dbPath[], char database[]);

/*
 * Function to dump records of a database
 */
void DbBrowse(DbEnv env){
	Data data;
	DbTxn txn;
	int error;
	int count = 0;

	//Initialize database	
	if(DbInit(env, &error) != 0){
		printf("Error opening database\n");
		exit(1);
	}

	//Call DbGet for all the records in database
	while(DbGet(env, &data, &txn, &error) != -1){
		data->data[data->len] = '\0';
		printf("%s\n\n", data->data);
		fflush(stdout);
		free(data->data);
		free(data);
		free(txn);
		count++;
	}
	
	printf("Total number of records: %d\n", count);
}

/*
 * Function to determine directory and database name from
 * a database path
 */
static void GetDbPath(char path[], char dbPath[], char database[])
{
	char *ptrptr;
	char *token;
	char *temp;
	char buf[MAX_PATH_LEN];
	

	token = strtok_r(path, "/", &ptrptr);
	if(path[0] == '/'){
		strcpy(dbPath, "/");
	}
	else{
	temp = strtok_r(NULL, "/", &ptrptr);

	//If database is in current directory
	if(temp == NULL){
		strcpy(dbPath, getcwd(buf, MAX_PATH_LEN));
		strcat(dbPath, "/");
		strcpy(database, path);
		return;
	}
		
	if(temp != NULL){
		strcpy(dbPath, token);
		strcat(dbPath, "/");
		token = temp;
	}		

	}

	while((temp = strtok_r(NULL, "/", &ptrptr)) != NULL){
		strcat(dbPath, token);
		strcat(dbPath, "/");
		token = temp;
	}

	strcpy(database, token);
}

int main(int argc, char *argv[])
{
	DbEnv env;
	char database[MAX_PATH_LEN];
	char dbPath[MAX_PATH_LEN];

	//If path not specified
	if(argc != 2){
		printf("%s: [database path]\n", argv[0]);
		return(0);
	}
	//Get database path and database name
	GetDbPath(argv[1], dbPath, database);

	//Initialize environment
	env = (DbEnv) malloc(sizeof(DbEnvStruct));
	env->databaseName = (char*) malloc(strlen(database));
	env->pathName = (char*) malloc(strlen(dbPath));
	strcpy(env->databaseName, database);
	strcpy(env->pathName, dbPath);
	env->numRecords = NUM_RECORDS;
	env->queueSize = QUEUE_SIZE;
	env->initialFileSize = FILE_SIZE;

	//Browse database
	DbBrowse(env);

	return(0);
}

