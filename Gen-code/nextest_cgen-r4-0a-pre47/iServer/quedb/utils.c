#include "utils.h"
#include "queue.h"
#include <string.h>

/*
 * Function to determine whether a string contains
 * only numbers
 */
int 
IsNumInString(char *str){
	unsigned int count;
	
	for(count = 0; count < strlen(str); ++count){

		//String does not contain digit, return false
		if(!isdigit(str[count])){
			return(0);
		}
	}
	
	//String contains only digits, return true
	return(1);
}

/*
 * Function to sort a integer array in ascending order.
 */
void
SortInt(int num[], int count){
	int i;
	int j;
	int temp;
	for(i = 0; i < count; ++i){
		for(j = i + 1; j < count; ++j){
			if(num[i] > num[j]){
				temp = num[i];
				num[i] = num[j];
				num[j] = temp;
			}
		}
	}
}

/*
 * Function to return a string having a integer padded with
 * leading zeros
 */
void
GetPaddedString(uint32_t num, uint32_t size, char* paddedStr){
	uint32_t count;
	char numStr[MAX_STRING_LEN];
	char *fn = "GetPaddedString";

	//Convert number into string
	sprintf(numStr, "%d", num);

	if(size < strlen(numStr)){
		NETERROR(MQUEDB, ("%s Number length is more than required padded string length", fn));
		strcpy(paddedStr, numStr);
		return;
	}
		

	//Prepend remaining elements of string with '0'
	for(count = 0; count < (size - strlen(numStr)); ++count){
		paddedStr[count] = '0';
	}
	paddedStr[count] = '\0';

	//Append number to string padded with '0'
	strcat(paddedStr, numStr);
}


/*
 * Function to return path of data files.
 */
int
GetFilePath(DbEnv env, char* path)
{

 //Generate data file path using path name and database name
 sprintf(path, "%s/%s", env->pathName, env->databaseName);

 return(0);
}
