#ifndef _LICENSE_IF_H_
#define _LICENSE_IF_H_

#include "timer.h"

#define FEATURE_NAME_LEN 20
#define MAX_FEATURES	 10 

extern 	int 	license_init(void);
extern 	int 	license_allocate(int n);
extern 	void	license_release(int n);
extern 	void 	license_display(void);
extern 	void 	nlm_setconfigport(int n);
extern 	int		nlm_getconfigport(void);
extern	int 	license_ChkExpiry(tid t);
extern 	int		nlm_getvport(void);
extern	int 	nlm_getUsedvport(void);
extern 	int 	nlm_initConfigPort(void);
extern 	int		nlm_getMRvport(void);
extern 	int		nlm_freeMRvport(void);
extern	int		nlm_getMaxRealms(void);
extern	int		nlm_getMaxRoutes(void);
extern	int		nlm_getMaxDynamicEP(void);
extern	int		nlm_isExpired(void);
extern	int		nlm_getInitTime(void);
extern	void	nlm_setInitTime(time_t);

extern 	int		h323Enabled(void);
extern	int		sipEnabled(void);
extern	int		fceEnabled(void);
extern 	int		radiusEnabled(void);
extern 	int		siptEnabled(void);
extern 	int		natEnabled(void);
extern 	int		scmEnabled(void);
extern 	int		registrarEnabled(void);
extern 	int		cacEnabled(void);
extern 	int		genEnabled(void);
extern 	char * 	nlm_getFeatureList(char flist[]);


/* Local prototypes for use within common */
extern int lm_getLicCount(char *release, char * version,time_t *pexpiry, char *macStr,int *pMaxCalls,int *pMaxMRCalls);
extern int lm_verify(unsigned char *msg,int len,unsigned char *sign,int siglen);
extern  int validateHwAddress(int hwlen, unsigned char *hwaddr);
extern  int validateHostid(char *hwaddr);


#endif  
