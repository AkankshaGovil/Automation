#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include "fileutils.h"
#include "srvrlog.h"

int
removeDir(const char *dirname)
{
	DIR 	*dirp;
	struct	dirent *entry, **result = 0;
	int		i, rc, numf = 100, maxent;
	int		flen, dirlen;
	char	**flist, *fname;

	if ( !(dirp = opendir(dirname)) ) {
		NETERROR(MCLI, ("opendir: %s\n", strerror(errno)));
		return -1;
	}

	dirlen = strlen(dirname) + 1;

	maxent = pathconf(dirname, _PC_NAME_MAX) + sizeof(struct dirent);
	flist = (char **)malloc(numf);
	entry = (struct dirent *)malloc(maxent);

	for(i = 0; 1 ;) {
#ifdef _POSIX_PTHREAD_SEMANTICS
		if ((rc = readdir_r(dirp, entry, result)) != 0) {
#else
		errno = 0;
		if (readdir_r(dirp, entry) == NULL) {
			if (errno == 0)
				break; 	// No more entries
#endif
			// Error occurred in readdir
			NETERROR(MCLI, ("readdir_r: %s\n", strerror(errno)));
			return -2;
		}

#ifdef _POSIX_PTHREAD_SEMANTICS
		if (!(*result)) {
			// No more entries;
			break;
		}
#endif

		if (i == numf) {
			// allocate more space for flist
			numf *= 2;
			flist = (char **)realloc((void *)flist, numf);
		}

		fname = entry->d_name;
		flen = dirlen + strlen(fname) + 1;

		// get the filename
		if ((strcmp(fname, ".") == 0) || (strcmp(fname, "..") == 0)) 
			continue; // ignore these files
		
		flist[i] = (char *)malloc(flen);
		snprintf(flist[i++], flen, "%s/%s", dirname, fname); 	
	}
	
	flist[i] = NULL;
	free(entry);

	for (i = 0; flist[i] != NULL; i++) {
		NETERROR(MCLI, ("Removing file %s\n", flist[i]));
		if ((rc = unlink(flist[i])) != 0) {
			NETERROR(MCLI, ("file %s, unlink: %s\n", flist[i], strerror(errno)));
		}
		free(flist[i]);
	}

	free(flist);

	// Finally remove the dir
	NETERROR(MCLI, ("Removing dir %s\n", dirname));
	if ((rc = rmdir(dirname)) != 0) {
		NETERROR(MCLI, ("dir %s, rmdir: %s\n", dirname, strerror(errno)));
		return -3;
	}

	return 0;
}
