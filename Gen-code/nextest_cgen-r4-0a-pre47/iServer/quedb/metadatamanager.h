#ifndef _METADATAMANAGER_H_
#define _METADATAMANAGER_H_
#include <stdio.h>
#include "queue.h"
#include <sys/types.h>
#include <dirent.h>

extern int ReadFileInfo(DbEnv env, int fd, FileInfo fileInfo); 
extern int DeleteFileInfo(Db db, FileInfo fileInfo, int* error );
extern int AddFileInfo(Db database, int*  error);
extern int AddMetadataEntry(DbEnv env, int value, int *offset, int *error);
extern int GetFileInfoList(Db database, int *error);
extern int OpenMetaDataFile(char *dbPath, int *fileDesc, int *error);
extern int IsValidName(DbEnv env, char name[]);
extern int GetNameFromIndex(DbEnv env, int index, char name[], int *error);
extern int FreeFileInfoListItems(List fileInfoList, int *error);
#endif
