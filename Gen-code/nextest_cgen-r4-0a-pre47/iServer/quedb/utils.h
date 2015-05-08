#ifndef _UTILS_H_
#define _UTILS_H_
#include "queue.h"
#include <ctype.h>
extern void GetPaddedString(uint32_t num, uint32_t size, char* paddedStr);
extern int GetFilePath(DbEnv env, char* path);
extern void SortInt(int num[], int count);
extern int IsNumInString(char *str);
#endif
