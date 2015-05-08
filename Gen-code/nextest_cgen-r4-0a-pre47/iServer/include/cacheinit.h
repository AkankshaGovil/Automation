#ifndef _CACHEINIT_H_
#define _CACHEINIT_H_

#include "serverp.h"
#include "ssip.h"

int LsMemPopulate (LsMemStruct *ms);
int LsMemStructReset (void);
int LsMemStructAtt (LsMemStruct *m);
int LsMemStructAttLocal (LsMemStruct *m);
int LsMemStructAttLocal (LsMemStruct *m);
int CallInit (LsMemStruct *m);
int CallInitLocal (LsMemStruct *m);
int CallInitLocal (LsMemStruct *m);
int ConfInit (LsMemStruct *m);
int SipCallCacheInit (LsMemStruct *m);
int RegCacheInit (LsMemStruct *m);
int SipTransCacheInitLocal (LsMemStruct *m);
int AlarmInit (LsMemStruct *m);
int RsdInit (LsMemStruct *m);
void CPCacheDestroyData (LsMemStruct *m);
void CPCacheInstantiate (LsMemStruct *m);
int CPInit (LsMemStruct *m);
void  CPBCacheDestroyData (LsMemStruct *m);
void CPBCacheInstantiate (LsMemStruct *m);
int CPBInit (LsMemStruct *m);
int IedgeInit (LsMemStruct *m);
void IedgeCacheDestroyData (LsMemStruct *m);
int IedgeCacheInstantiate (LsMemStruct *m);
int UpdateListInit (LsMemStruct *m);
void VpnCacheDestroyData (LsMemStruct *m);
void VpnCacheInstantiate (LsMemStruct *m);
int VpnInit (LsMemStruct *m);
void VpnGCacheDestroyData (LsMemStruct *m);
void VpnGCacheInstantiate (LsMemStruct *m);
int VpnGInit (LsMemStruct *m);
int TriggerInit (LsMemStruct *m);
int RealmCacheInstantiate (LsMemStruct *m);
int RealmInit (LsMemStruct *m);
int RealmCacheDestroyData (LsMemStruct *m);
int IgrpCacheInstantiate (LsMemStruct *m);
int IgrpInit (LsMemStruct *m);
int IgrpCacheDestroyData (LsMemStruct *m);
int CacheCallLegCmp (const void *v1, const void *v2);
int TriggerPopulate (LsMemStruct *m, DB dbin);
int RealmPopulate (LsMemStruct *m, DB dbin);
int IgrpPopulate (LsMemStruct *m, DB dbin);
int CacheAttach (void);
int CacheDetach (void);
int SipCallCacheInit (LsMemStruct *m);
int RegCacheInit (LsMemStruct *m);
int SipTransCacheInitLocal (LsMemStruct *m);
int CacheInit (void);
int UpdateIedgeDynamicInfo (InfoEntry *dinfo, InfoEntry *sinfo);
void ResetIedgeDbFields (CacheTableInfo *cacheInfo, short unsigned int startup);
int ResetIedgeFields (CacheTableInfo *cacheInfo, short unsigned int startup);
int UpdateIgrpDynamicInfo (IgrpInfo *dinfo, IgrpInfo *sinfo);
int UpdateVnetDynamicInfo (VnetEntry *dEntry, VnetEntry *sEntry);
int SipCompareUrls (header_url *src, header_url *dst);
int InitCfgFromCfgParms (CfgParms *cfgParms);
void allocCallStats (void);
int LsMemStructInitLocal(LsMemStruct *ms);
int InitCfgParms(CfgParms *cfgParms);
	
	

#endif /* _CACHEINIT_H_ */
