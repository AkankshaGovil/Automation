#ifndef	_INCLUDE_FILEUTILS_H_
#define	_INCLUDE_FILEUTILS_H_

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

extern int removeDir(const char *dirname);

#endif	// _INCLUDE_FILEUTILS_H_
