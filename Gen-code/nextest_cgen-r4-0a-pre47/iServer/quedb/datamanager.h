#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_
#include "queue.h"
#include <string.h>

extern int ReadRecord(DbEnv env, FileInfo fileInfo, uint32_t *readOffset, DataInfo *dataInfo, int* error);
extern int WriteRecord(DbEnv env, FileInfo fileInfo, Data data, uint32_t* newOffset, int* error);
extern int DeleteRecord(DbEnv env, FileInfo fileInfo, uint32_t newOffset, int* error);
extern int OpenDataFile(DbEnv env, int fileIndex, int *fileDesc, int* error);

extern int InitDataInfoList(Db database, int *error);
extern int GetWriteOffset(Db database, int *error);
extern int GetRecordCount(DbEnv env, int fd, int *recordCount, int *activeRecordCount, uint32_t *writeOffset, int *error);
extern int PopulateDataInfoList(Db database, int* error);
extern int DeleteDataFile(DbEnv env, int fileIndex, int* error);
extern int FreeDataInfo(DataInfo item, int *error);
extern int FreeDataInfoListItems(List dataInfoList, int *error);
#endif
