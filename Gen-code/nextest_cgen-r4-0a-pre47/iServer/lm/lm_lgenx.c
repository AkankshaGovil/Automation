#include <stdio.h>
#include <stdlib.h>
#include "serverp.h"


LsMemStruct lsMemStruct;
LsMemStruct *lsMem;

#define MAX_LINE_SIZE 200

extern char *xmlLicenseFile;

int main(int argc, char **argv)
{
	lsMem = &lsMemStruct;

	xmlLicenseFile = "iserverlc.xml";

	license_init();
	lsMem->termTime=0;		/* We are trying to correct the sig, not kill the license~ */
	writeXmlLicense();

	return 0;
}

/* Prevent unnecessary bloat by defining these symbols */
int LockReleaseLock (Lock *x) {return 0;}
int LockGetLock (Lock *x, int mode, int block) {return 0;}
cache_t confCache;
int CacheReleaseLocks (cache_t c) {return 0;}
int CacheAttach(void) {return 0;}
int CacheDetach(void) {return 0;}
int CacheGetLocks (cache_t x, int mode, int block) {return 0;}

