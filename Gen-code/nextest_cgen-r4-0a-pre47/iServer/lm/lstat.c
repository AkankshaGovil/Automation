#include <stdio.h>
#include <sys/syslog.h>
#include <limits.h>
#include "spversion.h"
#include "serverp.h"
#include "mem.h"
#include "srvrlog.h"
#include "license.h"
#include "licenseIf.h"
#ifdef HEADER_BIO_H
#undef HEADER_BIO_H
#endif
#include "bn.h"
#include "sha.h"
#include "dsa.h"

#define TRUE    1
#define FALSE   0
#define OK      0




LsMemStruct lsMemStruct ,*lsMem;
cache_t                 confCache;



#include <stdio.h>
#include <sys/time.h>
#include "license.h"

main()
{
	int 	usedlic,maxCalls;
	time_t	expiry_time;
	char 	macstr[MAC_ADDR_LEN];
	char 	flist[MAX_FEATURES*FEATURE_NAME_LEN] = {0};
	int		i;

	lsMem = &lsMemStruct;
	NetSyslogOpen("lstat", NETLOG_ASYNC);
	netLogStruct.flags |= -1;
	for(i=0;i<MNETLOGMAX;++i)
	{
		NetLogStatus[i] = 4;
	}


	lm_getLicCount(FEATURE,LIC_VERS,&expiry_time,macstr,&maxCalls);

	printf("Max Calls\t%d\n"
	"Expiry\t\t%s",maxCalls,ctime(&expiry_time));
	printf("mac\t\t%s\n",macstr);
	printf("Features\t%s\n",nlm_getFeatureList(flist));


	return 0;
}


int ShmAttached(){return 1;}
int CacheGetLocks(cache_t cache, int mode, int block){};
int CacheReleaseLocks(cache_t cache){};
int CacheAttach(){};
int CacheDetach(){};


