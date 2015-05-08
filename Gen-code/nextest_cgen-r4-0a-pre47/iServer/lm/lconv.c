#include "spversion.h"
#include "serverp.h"
#include "license.h"
#include "licenseIf.h"
#include <sys/time.h>

LsMemStruct lsMemStruct, *lsMem;
cache_t                 confCache;

extern char	*xmlLicenseFile;
extern char	*licenseFile;


int main()
{
	int 	maxCalls,maxMRCalls;
	time_t	expiry_time;
	char 	macstr[MAC_ADDR_LEN];
	char 	flist[MAX_FEATURES*FEATURE_NAME_LEN] = {0};
	int		i;

	lsMem = &lsMemStruct;

	xmlLicenseFile	= "iserverlc.xml";
	licenseFile	= "iserver.lc";

	lsMem->nlic = lm_getLicCount(FEATURE, LIC_VERS, &lsMem->expiry_time, lsMem->macstr,
							&lsMem->maxCalls,&lsMem->maxMRCalls);

	printf("Max Calls\t%d\nMax MR Calls\t%d\nExpiry\t\t%s", lsMem->maxCalls, lsMem->maxMRCalls, lsMem->expiry_time==0?"never":ctime(&lsMem->expiry_time));
	printf("mac\t\t%s\n", lsMem->macstr);
	printf("Features\t%s\n",nlm_getFeatureList(flist));


	writeXmlLicense();

	return 0;
}


int ShmAttached(){return 1;}
int CacheGetLocks(cache_t cache, int mode, int block){ return(0); }
int CacheReleaseLocks(cache_t cache){ return(0); }
int CacheAttach(){ return(0); }
int CacheDetach(){ return(0); }
int LockGetLock(Lock*l,int m,int b){ return(0); }
int LockReleaseLock(Lock*l){ return(0); }

